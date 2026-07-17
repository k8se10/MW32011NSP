# Patch Notes

All notable changes to this project, per release. See `re_notes/` for the full
vulnerability-research and reverse-engineering trail behind each entry, and
`CLAUDE.md` (game install root) for the complete technical plan.

---

## Unreleased

### Investigated-not-resolved
- **Plutonium `iw5sp.exe` P2P vulnerability cross-check: CONFIRMED same bug
  present, byte-for-byte identical machine code (2026-07-17, later
  session).** Followed up the earlier Plutonium binary comparison by
  independently importing and fully analyzing Plutonium's own `iw5sp.exe`
  from scratch (not assuming any retail address carries over, since it's a
  differently-compiled binary). Result: every instruction in the vulnerable
  P2P-receive function is unmodified between retail and Plutonium's build —
  not just the same bug class, the exact same machine code, despite the two
  binaries differing elsewhere. Strongest available confirmation that this
  is a shared-binary issue, not something either distribution introduced
  independently.
  - **Bonus finding, significant enough to flag on its own**: this pass
    surfaced a real, substantial, actively-used raw-socket (`WS2_32.dll`)
    networking subsystem in `iw5sp.exe` — dozens of real call sites across
    distinct functions, not vestigial — entirely unexplored by any research
    so far, and structurally separate from the Steamworks P2P path this
    project has focused on. Strong candidate for the next research pass.
  - Exact addresses/disassembly stay in the gitignored internal notes per
    this project's redaction policy (still unpatched). Full detail
    (redacted appropriately) in `re_notes/vulnerability_research.md`.
- **Direct cross-check against Plutonium's own installed binaries
  (2026-07-17, later session), plus disclosure-language and fact
  corrections.** Prompted by the user's own realization that a client-side
  bug in a byte-identical binary implies Plutonium isn't fully insulated
  either — refined the doc's framing from a blanket "Plutonium isn't safe"
  claim to the precise, defensible one: a vulnerable client-side code path
  exists in the shared binary, and whether Plutonium's own deployment makes
  it unreachable in practice is a separate, unconfirmed question this
  project hasn't investigated. Then did the actual direct verification:
  - Re-confirmed `iw5mp.exe` byte-identical via direct SHA256 (not just
    citing the earlier claim) — this alone proves the confirmed MP
    client-side finding is present in Plutonium's client too, no further
    work needed for that specific question.
  - Found and corrected two factual errors surfaced by this check: an
    earlier claim that `iw5sp.exe` has no `WS2_32.dll` import was wrong
    (verified via `dumpbin /imports`, both retail and Plutonium import it
    identically — flags a new, unexplored raw-socket attack surface,
    doesn't overturn the Steamworks-P2P conclusion which was reached via
    call-site tracing); and a previously-recorded "~175KB smaller" size
    difference for Plutonium's `iw5sp.exe` was wrong — actual delta is
    2,320 bytes, the ~175KB figure was very likely a byte-difference COUNT
    mistaken for a file-size difference. Also corrected in the sibling
    `MW32011NCP` project's own notes (cross-reference policy).
  - Launched a focused pass to determine whether Plutonium's own,
    differently-compiled `iw5sp.exe` still has the confirmed P2P
    receive-path vulnerability — in progress, not yet resolved as of this
    entry; full detail (redacted while unresolved) in
    `re_notes/vulnerability_research.md` and the internal notes.
- **Four-fork netcode sweep across both binaries (2026-07-17, later
  session).** Prompted by reconciling "Activision patched Steam-Auth" against
  Plutonium's continued claims that vanilla Steam MW3 is unsafe — found the
  two aren't about the same bug (see `re_notes/vulnerability_research.md`'s
  "Reconciling..." section for the community-documented "MemberJoin RCE"
  citation). Independently swept both binaries' netcode for the same bug
  class and found:
  - **A second, currently-unpatched candidate vulnerability in `iw5mp.exe`**,
    in what looks like a compressed/fragmented message reassembly routine —
    an out-of-bounds write at an attacker-controlled offset, same
    "checked but not enforced" shape as the first finding, plausibly the
    real Huffman/CVE-2018-10718 bug class. A third, structurally similar,
    not-yet-fully-confirmed candidate also found.
  - **A separate, genuinely server-side per-client dispatcher subsystem
    located in `iw5mp.exe`** (the one call site audited there is safe) —
    the most plausible home for the community-reported "MemberJoin RCE,"
    flagged as the strongest lead for the "malicious client attacks the
    hosting player" direction, which no finding so far actually covers.
  - **The original `iw5mp.exe` finding's reachability was corrected**: both
    client-side (malicious server → connecting client) and via a malicious
    demo file — NOT reachable from a malicious client against a hosting
    player, as originally guessed from structure alone.
  - **A new, independently confirmed, currently-unpatched vulnerability
    found in `iw5sp.exe`'s Spec-Ops/Survival co-op networking** (Steamworks
    P2P receive path) — notably has no bounds check at all, not even a
    logged-but-unenforced one, and is confirmed reachable continuously
    (once per frame) during any active P2P session with a malicious peer.
  - **A real DoS/reflection-amplification gap found in `iw5mp.exe`'s
    pre-auth OOB dispatcher**: zero rate-limiting anywhere in that table,
    with `getinfo` showing a ~60-70x request/reply size factor — the same
    shape CoD4x_Server's `SVC_RateLimit` was built to close. Concrete Phase
    2 target.
  - **Corrected a factual error in this project's own prior research**: a
    previous claim that `iw5sp.exe` had its own "connection/challenge-
    adjacent code" at two specific addresses was wrong — both were
    unrelated unlock-notification code, a stale carry-forward of an already-
    documented false lead. Retracted.
  - Exact addresses/disassembly for every still-unpatched item above stay in
    the gitignored `re_notes/INTERNAL_vulnerability_research.md` per this
    project's redaction policy — no fixes written yet, this was a research
    pass. Full detail (redacted appropriately) in
    `re_notes/vulnerability_research.md`.
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
