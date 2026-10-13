<#
.SYNOPSIS
    Builds and runs upstream's test_feature against the library as SolEngine builds it.

.DESCRIPTION
    Step 1 of the release check in docs/testing.md. MSVC and clang-cl, Debug and
    Development, through the Visual Studio 2022 generator as SolEngine's presets use it.
    Each run's output is kept in check/Build/<toolset>-<configuration>.log.

    A run passes when the suite exits 0 and the library reports AS_NO_EXCEPTIONS: a
    definition that stopped reaching the library would otherwise pass silently, with the
    tests it guards back on.

.PARAMETER Toolset
    MSVC, ClangCL, or both (the default).

.PARAMETER Configuration
    Debug, Development, or both (the default).
#>
param(
    [ValidateSet('MSVC', 'ClangCL')]
    [string[]] $Toolset = @('MSVC', 'ClangCL'),

    [ValidateSet('Debug', 'Development')]
    [string[]] $Configuration = @('Debug', 'Development')
)

$ErrorActionPreference = 'Stop'

$Source  = $PSScriptRoot
$Build   = Join-Path $PSScriptRoot 'Build'
# The suite reads and writes script files relative to where upstream's projects put it.
$RunDir  = Join-Path $PSScriptRoot '..\sdk\tests\test_feature\bin' | Resolve-Path

# What was tested, for the summary: a tag is cut only on a pass, so the pass has to
# name the commit it is for.
$Repo     = Join-Path $PSScriptRoot '..' | Resolve-Path
$Commit   = git -C $Repo rev-parse HEAD
$SdkDirty = git -C $Repo status --porcelain -- sdk

$Results = @()

foreach ($T in $Toolset) {
    $Tree = Join-Path $Build $T

    $ConfigureArgs = @('-S', $Source, '-B', $Tree, '-G', 'Visual Studio 17 2022', '-A', 'x64')
    if ($T -eq 'ClangCL') { $ConfigureArgs += @('-T', 'ClangCL') }

    & cmake @ConfigureArgs
    if ($LASTEXITCODE -ne 0) {
        foreach ($C in $Configuration) {
            $Results += [pscustomobject]@{ Run = "$T-$C"; Result = 'CONFIGURE FAILED' }
        }
        continue
    }

    foreach ($C in $Configuration) {
        $Name = "$T-$C"
        Write-Host "=== $Name ===" -ForegroundColor Cyan

        & cmake --build $Tree --config $C -- /m /v:minimal
        if ($LASTEXITCODE -ne 0) {
            $Results += [pscustomobject]@{ Run = $Name; Result = 'BUILD FAILED' }
            continue
        }

        $Exe = Join-Path $Tree "$C\test_feature.exe"
        $Log = Join-Path $Build "$Name.log"

        Push-Location $RunDir
        try {
            # Through cmd so the suite's stderr lands in the log beside its stdout,
            # without PowerShell 5.1 wrapping each line as an error record.
            cmd /c "`"$Exe`" > `"$Log`" 2>&1"
            $Exit = $LASTEXITCODE
        }
        finally {
            Pop-Location
        }

        $Options = Select-String -Path $Log -Pattern '^AngelScript options:' | Select-Object -First 1
        if ($Exit -ne 0) {
            $Result = "FAILED (exit $Exit)"
        }
        elseif (-not $Options -or $Options.Line -notmatch '\bAS_NO_EXCEPTIONS\b') {
            $Result = 'FAILED (library does not report AS_NO_EXCEPTIONS)'
        }
        else {
            $Result = 'passed'
        }

        Write-Host "$Name $Result, log: $Log"
        $Results += [pscustomobject]@{ Run = $Name; Result = $Result }
    }
}

$Results | Format-Table -AutoSize | Out-String | Write-Host

if ($SdkDirty) { Write-Host "At $Commit, with uncommitted changes under sdk/" -ForegroundColor Yellow }
else           { Write-Host "At $Commit, sdk/ clean" }

if ($Results | Where-Object { $_.Result -ne 'passed' }) { exit 1 }
exit 0
