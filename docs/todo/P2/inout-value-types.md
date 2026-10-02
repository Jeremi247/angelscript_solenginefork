# `&inout` for a value type without unsafe references

**Ask** — a registered value type may cross as `&inout` without `asEP_ALLOW_UNSAFE_REFERENCES`. Today the builder requires `&inout` on an object with `addref` and `release` or `asOBJ_NOCOUNT`, which a plain value type has neither of. SolEngine turned the property on for the whole engine to pass a written component as `&inout` ([SolEngine 0083](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0083-a-written-component-crosses-as-inout.md)).

**Open**
- What the narrower rule is. The unsafe-references property exists because a reference into an object can outlive the object; a value type held by the caller for the length of the call cannot, and stating that precisely is the work.
- What the engine-wide property costs SolEngine today. 0083 bounds the aliasing risk to bindings taking `T &`, so the gain may be small.

**Read**
- [SolEngine 0083](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0083-a-written-component-crosses-as-inout.md) — the property, and the cost it bounds
- [SolEngine 0079](https://github.com/Jeremi247/SolEngine/blob/main/docs/decisions/0079-deducing-a-binding-declaration.md) — where the builder's requirement was found
- `sdk/angelscript/source/as_builder.cpp` — the check
