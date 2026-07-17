# Security Policy

## What this project is

This is a defensive security project: it studies Call of Duty: Modern Warfare 3
(2011, IW5 engine)'s netcode for real, exploitable vulnerabilities, and ships
fixes for them, primarily to protect players joining public/untrusted servers
(applies to both `iw5sp.exe` — Campaign/Survival/Spec-Ops co-op — and
`iw5mp.exe` — Multiplayer). It exists because this engine family has a real,
documented history of network-reachable bugs, and Steam's VAC (confirmed active
on `iw5mp.exe`) is aimed at memory-cheat detection, not packet-level/protocol
attacks — a different threat model this project exists to actually cover.

**This creates a two-sided responsibility that doesn't exist for a typical
project's `SECURITY.md`**: this repository's own commit history and `re_notes/`
are, by necessity, a running record of real vulnerabilities in the base game —
not something to hide, since fixing them requires documenting them precisely
enough for a contributor to verify a fix actually works, but also not something
to turn into a how-to-exploit resource. See "Responsible disclosure" below for
how this project handles that tension.

## Supported Versions

Pre-alpha software under active development, same as the sibling `MW32011NCP`
project. Only the most recent release is supported.

| Version | Supported |
| ------- | --------- |
| Latest release (see [Releases](../../releases)) | ✅ |
| Older releases | ❌ |

## Two categories of security report

**1. A vulnerability in the BASE GAME's netcode this project hasn't found/fixed
yet.** This is the primary thing this project wants to hear about — report it
per "Responsible disclosure" below, not as a public issue.

**2. A bug in THIS PROJECT's OWN patch code** (memory-safety issues, a fix that
doesn't actually close the vulnerability it claims to, a supply-chain concern
with a release artifact). Same categories as any security-sensitive C/C++
project — report the same way, see below.

## Responsible disclosure

**Do not open a public GitHub issue for a netcode vulnerability, in this game
or any other.** Email **k8se10@gmail.com** instead. Include:

- What you found and why you believe it's exploitable (a crash is not
  automatically a security issue — see "Not in scope" below)
- Enough detail to reproduce and verify a fix, but please avoid attaching a
  ready-to-run exploit/PoC tool in the initial report — a written description
  and/or a minimal packet capture is preferred; if a working PoC is genuinely
  necessary to demonstrate impact, say so and we'll agree on how to share it
  safely
- Whether you're aware of this already being exploited in the wild (this
  changes urgency significantly)
- Which release/commit you tested against

You should get an acknowledgment within a few days. This is a solo,
from-scratch project worked on outside full-time hours — response and fix time
will vary, but security reports are prioritized over regular feature work.
Once a fix ships, the vulnerability and fix get documented in `re_notes/` and
`PATCHNOTES.md` in enough detail for the community to verify the fix, following
the same "document every last detail" standard as the sibling `MW32011NCP`
project's `CODE_STANDARDS.md` — but a genuinely severe, actively-exploited
finding may have its full technical detail held back for a short period after
the fix ships, to give the community running-server operators time to update
before the exact mechanism is public. This project does not follow a rigid
fixed-day embargo — timing is judged per finding, biased toward disclosure once
a fix is available and verified, not toward secrecy.

## Not in scope

- Ordinary gameplay bugs, crashes from normal (non-adversarial) play, or
  anything specific to a single player's own session with no broader
  implication for other players/servers
- Memory/aimbot-style cheats — that's VAC's domain (already covers this to
  whatever extent it does), and a different threat model from what this
  project addresses
- Vulnerabilities in the base game unrelated to netcode (e.g. a local save-file
  parsing bug with no network reach) — still worth reporting, but lower
  priority for this specific project's scope

## Scope note: this is not the game itself

This project has no affiliation with Activision, Infinity Ward, or Call of
Duty: Modern Warfare 3 itself. It is an independent, unofficial security patch.
Activision/Infinity Ward are not this project's disclosure channel and
reporting a vulnerability here does not constitute reporting it to them.
