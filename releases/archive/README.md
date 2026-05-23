# CIA archive

Tracked copies of installable `.cia` builds for reference and CIA packaging work.

| File | Source | Notes |
|------|--------|-------|
| `3DS-Random-Game-Launcher-v18.cia` | [GitHub Release v18](https://github.com/selloa/3DS-Random-Game-Launcher/releases/tag/v18) | Newest **published** CIA (Sep 2025, v18 beta) |
| `3DS-Random-Game-Launcher-pre-cleanup.cia` | Git history (`b8685f7^`) | Older experimental build from repo root (187 KB) |

## Not recoverable

These were **untracked** local builds in `CBuilder3DS/` and were permanently removed by `git clean` during submodule cleanup:

- `3DS-Random-Game-Launcher v19.cia`
- `3DS-Random-Game-Launcherv20.cia`
- `3DS-Random-Game-Launcher.cia` (unversioned copy in CBuilder3DS)

They were never committed to git. Rebuild with the CIA workflow in [tools/README.md](../tools/README.md) to replace them.

## Usage

Install on CFW with FBI or similar. For new builds, output to `dist/` (gitignored) and copy here only when you want a tracked archive copy.
