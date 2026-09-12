# MW3 Netcode Security Patch — moved into MW32011NCP

**This repository is archived.** Its work continues at
[MW32011NCP](https://github.com/k8se10/MW32011NCP), under
[`security/`](https://github.com/k8se10/MW32011NCP/tree/main/security).

## What happened

2026-09-12: `MW32011NCP` (the native controller/enhancement mod for the
same game) was redefined from "Native Controller Project" to "Native
Community Patches" — its identity expanded to formally cover controller
input, the visual-enhancement suite, and native netcode security patching
as three real, shipped scopes of one project, rather than this staying a
separate sibling effort.

This repository's entire commit history (every finding, every fix, the
original vendor security-disclosure record submitted to Activision) was
merged into `MW32011NCP` intact via a `git subtree` merge — a real,
history-preserving move, not a fresh copy. Nothing here is lost; the two
repos' git graphs are genuinely connected at that merge point.

Nothing about the actual security work changed: the same three confirmed
netcode vulnerabilities are fixed the same way, the standalone DLL still
builds, and the plugin variant still ships built into `MW32011NCP` by
default via its existing "greenlit" allowlist. Only the repository this
work lives in changed.

## Where to go instead

- Source, findings, and current fix status:
  [`MW32011NCP/security/`](https://github.com/k8se10/MW32011NCP/tree/main/security)
- Full decision record for the merge itself: `MW32011NCP`'s own
  `CLAUDE.md`, 2026-09-12 Version Timeline entry.
- If you also want native controller support for this game: the
  [`MW32011NCP` repo root](https://github.com/k8se10/MW32011NCP) itself.

This repository stays published, read-only, for historical continuity
(existing links, stars, and the original disclosure record) — it is not
deleted, and nothing in its history has been altered.
