// AS_SOL_SRW_LOCK: docs/decisions/0012-the-read-write-lock-is-an-srw-lock.md

#include "FeatureTest.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

namespace
{
    using FClock = std::chrono::steady_clock;

    // Waits until Done returns true or the time is up, and returns Done's last answer.
    template <typename F>
    bool WaitFor(F Done, int Milliseconds)
    {
        const FClock::time_point Until = FClock::now() + std::chrono::milliseconds(Milliseconds);
        while (!Done())
        {
            if (FClock::now() > Until)
                return false;
            std::this_thread::yield();
        }
        return true;
    }
}

FT_TEST(SrwLockIsReported)
{
    FT_CHECK(FeatureTest::IsReported("AS_SOL_SRW_LOCK"));
}

// Upstream's lock is a semaphore of ten reader slots, so an eleventh reader waits for
// one of the ten to leave. They all wait for each other here, and upstream never ends.
FT_TEST(SrwLockAdmitsMoreThanTenReaders)
{
    FeatureTest::FEngine E;

    const int Readers = 16;
    std::atomic<int> Holding{ 0 };
    std::atomic<bool> Leave{ false };

    std::vector<std::thread> Threads;
    for (int n = 0; n < Readers; ++n)
        Threads.emplace_back([&] {
            asAcquireSharedLock();
            ++Holding;
            WaitFor([&] { return Leave.load(); }, 10000);
            asReleaseSharedLock();
        });

    FT_CHECK(WaitFor([&] { return Holding.load() == Readers; }, 5000));
    Leave = true;
    for (std::thread& Thread : Threads)
        Thread.join();
}

FT_TEST(SrwLockExcludesReadersWhileWriting)
{
    FeatureTest::FEngine E;

    std::atomic<bool> Read{ false };
    asAcquireExclusiveLock();
    std::thread Reader([&] {
        asAcquireSharedLock();
        Read = true;
        asReleaseSharedLock();
    });

    FT_CHECK(!WaitFor([&] { return Read.load(); }, 100));
    asReleaseExclusiveLock();
    FT_CHECK(WaitFor([&] { return Read.load(); }, 5000));
    Reader.join();
}

// Every take of the engine's lock, from threads reading while one builds, discards and
// collects. With asserts on, a take by a thread already holding the lock asserts.
FT_TEST(SrwLockEngineWorkloadUnderReaders)
{
    FeatureTest::FEngine E;
    E.Engine->SetUserData(&E, 1000);

    std::atomic<bool> Stop{ false };
    std::atomic<int> WrongUserData{ 0 };
    std::vector<std::thread> Readers;
    for (int n = 0; n < 4; ++n)
        Readers.emplace_back([&] {
            while (!Stop)
            {
                if (E.Engine->GetUserData(1000) != &E)
                    ++WrongUserData;
                asUINT Count = E.Engine->GetModuleCount();
                (void)E.Engine->GetModuleByIndex(Count);
            }
        });

    const char* const Script =
        "class Node { Node@ Next; }\n"
        "void Cycles() { for (int n = 0; n < 20; n++) { Node a; @a.Next = Node(); @a.Next.Next = a; } }\n";

    bool Built = true;
    for (int Round = 0; Round < 20 && Built; ++Round)
    {
        asIScriptModule* Module = E.Build("SrwLock.as", Script, Round % 2 ? "A" : "B");
        Built = Module != nullptr;
        if (!Built)
            break;
        Module->SetUserData(&E, 1000);
        FT_CHECK(Module->GetUserData(1000) == &E);
        FT_CHECK(E.Run(Module, "void Cycles()") == asEXECUTION_FINISHED);
        Module->Discard();
        E.Engine->GarbageCollect(asGC_ONE_STEP);
    }
    E.Engine->GarbageCollect(asGC_FULL_CYCLE);

    Stop = true;
    for (std::thread& Thread : Readers)
        Thread.join();

    FT_CHECK(Built);
    FT_CHECK(WrongUserData == 0);
}
