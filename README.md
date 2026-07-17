# MW3 Netcode Security Patch (MW3NSP)

**Status: PLANNING — no code written yet (as of 2026-07-17).** This is a sibling
project to [`MW32011NCP`](../MW32011NCP) (the native controller mod for this same
game) — see `CLAUDE.md` at the game install root for the full technical plan this
README summarizes.

A defensive security patch for Call of Duty: Modern Warfare 3 (2011, IW5 engine) —
finding and fixing real, exploitable vulnerabilities in this game's netcode. **This
engine generation has a real, documented history of network-reachable
buffer-overflow bugs, and Steam's VAC (confirmed active on `iw5mp.exe`) does not
protect against any of them** — VAC is signature-based cheat detection, a
completely different threat model from a malicious server or peer sending crafted
packets. That gap is why this project exists.

## Scope

- **Primary goal: protect players joining public/untrusted servers** — client-side
  hardening, not primarily a dedicated-server-operator tool (though server-side
  findings that help operators are a natural byproduct).
- **Covers both target binaries**: `iw5mp.exe` (Multiplayer) and `iw5sp.exe`
  (Campaign/Survival/Spec-Ops — the co-op networking path shares the same
  vulnerability surface as MP, confirmed by the project owner, not just a Campaign
  side-effect).
- Same "SP and MP are separately-built binaries, RE'd independently" principle as
  the sibling `MW32011NCP` project — see `CLAUDE.md` §10.8 (shared across both
  projects).

## What's already been found (research only, not yet fixed)

Full detail and citations in `re_notes/vulnerability_research.md`. Summary:

| Finding | MW3 status | Severity |
|---|---|---|
| CVE-2018-20817 ("Steam-Auth") | Explicitly named, CVSS 9.8 | Critical — public RCE PoC exists |
| CVE-2018-10718 ("Huffman") | Same engine-family bug class, MW2-confirmed | Critical if applicable (CVSS 10.0 for MW2) |
| MW3ProtocolExploit (2019) | MW3-specific, client-side, **confirmed still unpatched at the actual bug** | Client crash/exploit via crafted server-name string — the clearest match for this project's scope |
| `SendP2PPacket` NULL crash (aluigi, 2012) | MW3 ≤1.9.453, current-build status unverified | Unknown until re-checked |
| GSC VM exploits | Not concretely documented publicly | Unknown — needs original research (Phase 3) |

**None of these have been independently re-verified against this project's own
copies of `iw5sp.exe`/`iw5mp.exe` yet** — that's the first real implementation
step, not an assumption to build on blindly.

## Planned architecture

Same proxy-DLL injection technique already proven working in the sibling
`MW32011NCP` project (a proxy `d3d9.dll` gets code execution inside the game
process), but hooking the real, vulnerable network-message-parsing functions
directly instead of input functions — a pre-hook that validates/bounds-checks
incoming data before the real, unsafe code path runs. See `CLAUDE.md`'s full
"ARCHITECTURE" section for the complete plan.

## Phased plan

1. **Patch the known, documented bugs** (the table above), starting with the
   MW3ProtocolExploit client-side bug as the clearest match for this project's
   scope.
2. **General hardening** against undiscovered bugs of the same class — rate
   limiting, stronger connection-challenge validation, broader bounds-checking
   audits — modeled on real precedent from `CoD4x_Server` (one engine generation
   earlier, same lineage, already ships exactly this class of hardening).
3. **GSC VM security research** — lower priority, genuinely under-documented
   publicly, needs original RE work rather than a literature search.

## A note on responsible disclosure

This project's own `re_notes/` is, by necessity, a running record of real
vulnerabilities in the base game — documented precisely enough to verify a fix
works, not as a how-to-exploit resource. See `SECURITY.md` for how this project
handles reporting a *new* finding, and the disclosure-timing judgment applied to
genuinely severe, actively-exploited issues.

## License

Same model as the sibling `MW32011NCP` project — see [`LICENSE`](LICENSE). Free to
use, modify, and fork; neither this project nor any derivative may be sold or
placed behind a paywall. Does not grant any rights to Call of Duty: Modern Warfare
3 itself — this is an unofficial, fan-made security patch, not affiliated with or
endorsed by Activision or Infinity Ward.

## Contributing

See [`CONTRIBUTING.md`](CONTRIBUTING.md) and [`CODE_STANDARDS.md`](CODE_STANDARDS.md)
before opening a PR — same production-ready, live-verified bar as the sibling
project, plus this project's own responsible-disclosure norms for anything
touching a real vulnerability (`SECURITY.md`).
