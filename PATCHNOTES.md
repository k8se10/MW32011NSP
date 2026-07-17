# Patch Notes

All notable changes to this project, per release. See `re_notes/` for the full
vulnerability-research and reverse-engineering trail behind each entry, and
`CLAUDE.md` (game install root) for the complete technical plan.

---

## Unreleased

### Investigated-not-resolved
- **Steam-Auth (CVE-2018-20817) does NOT reproduce against this project's own
  retail `iw5mp.exe` (2026-07-17, later session).** Full decompile +
  disassembly of the real `"steamauth"` OOB packet handler (`FUN_005704b0`,
  reached via the real dispatcher `FUN_005763f0`) found a correct, sound
  length check (unsigned comparison, exactly matching the destination
  buffer's 2048-byte size, and correctly rejecting a sign-extended negative
  length rather than being bypassed by one) guarding the copy, with actual
  ticket validation delegated entirely to Valve's own Steamworks SDK
  (`SteamGameServer()`) rather than parsed by MW3's own code. This retail
  Steam build is very likely a post-fix build (the CVE itself notes a fix
  existed for versions before 2015-08-11). **There is no fix to write at this
  location** — this candidate is closed as a non-issue, not implemented as a
  patch. Full trail in `re_notes/vulnerability_research.md` and
  `re_notes/INTERNAL_vulnerability_research.md`. Next candidates to verify:
  Huffman (CVE-2018-10718), the MW3-specific 2019 protocol exploit, and
  PartyHost/joinParty (CVE-2019-20893).
- Also added a project-wide **redaction policy** (`SECURITY.md`, addendum in
  `CODE_STANDARDS.md`): exact addresses of still-unpatched vulnerable code
  stay in a gitignored `re_notes/INTERNAL_*.md` counterpart to each public
  doc, never committed, until a fix ships — everything else (CVEs,
  architecture, methodology) stays fully public. Repo itself remains public;
  this is a content-level discipline, not an access-control change.

### Docs
- **Project planning complete, scaffolding created (2026-07-17).** No code
  written yet. Established: scope (client-side hardening, both `iw5sp.exe`
  co-op and `iw5mp.exe` MP), a research-backed vulnerability list (two real
  CVEs, one explicitly naming MW3; one MW3-specific still-unpatched
  client-side exploit; one 2012 crash of unconfirmed current-patch-status; GSC
  VM flagged as under-documented and requiring original research), a phased
  implementation plan (patch known bugs → general hardening → GSC VM
  research), and the planned architecture (same proxy-DLL injection technique
  as the sibling `MW32011NCP` project, targeting the vulnerable network-
  parsing functions instead of input functions). Full detail in `CLAUDE.md`
  and `re_notes/vulnerability_research.md`.
- **First research/verification batch (2026-07-17, later session).** Seven
  parallel research forks: partial verification of the original 3
  vulnerabilities against this project's own binaries (found the real OOB
  packet dispatcher and a strong candidate handler for the Steam-Auth CVE,
  `FUN_005704b0` in `iw5mp.exe`, not yet confirmed vulnerable), a fourth
  candidate vulnerability found (CVE-2019-20893, same missing-length-check bug
  class), a real netcode architecture map (the OOB dispatcher and its full
  command table), CoD4x_Server's actual hardening code extracted in full as a
  concrete Phase 2 blueprint (real challenge-response/rate-limiting/infostring-
  fix code, not just descriptions), GSC VM security research (genuinely
  inconclusive, Phase 3's low-priority framing confirmed reasonable), a
  significant threat-model finding (**MW3 uses a real listen-server/host-
  migration model, meaning an ordinary player's own machine can run the
  vulnerable server-side code whenever they're the lobby host** — scope now
  explicitly includes protecting the hosting player from a malicious peer,
  not just a connecting client from a malicious server), and an investigation
  into Plutonium's own claimed fix for these same vulnerabilities — their
  client binary is confirmed byte-identical to retail, and forum/staff
  statements suggest their real mitigation is architectural (steering play to
  Plutonium-operated dedicated servers instead of the vulnerable player-hosted
  listen-server model) and protocol-level (bypassing Steam's ticket-auth path
  server-side rather than patching the buggy parser) — a reasonable inference,
  not independently confirmed. Full detail in `CLAUDE.md` and
  `re_notes/vulnerability_research.md`.

---
