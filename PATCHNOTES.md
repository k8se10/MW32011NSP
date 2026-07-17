# Patch Notes

All notable changes to this project, per release. See `re_notes/` for the full
vulnerability-research and reverse-engineering trail behind each entry, and
`CLAUDE.md` (game install root) for the complete technical plan.

---

## Unreleased

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

---
