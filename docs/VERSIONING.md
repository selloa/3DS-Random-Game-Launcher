# Versioning

This project uses [Semantic Versioning 2.0.0](https://semver.org/) (`MAJOR.MINOR.PATCH`).

The **single source of truth** is the [`VERSION`](../VERSION) file at the repository root (plain text, one line, no `v` prefix).

## Current version

See [`VERSION`](../VERSION). Builds and release tags use the `v` prefix (e.g. `0.1.9` → tag `v0.1.9`).

## Legacy integer versions

Early releases used auto-incrementing integers (`v1`, `v18-beta`, …) from `.build_number`. Those map to semver **0.1.x** during the transition:

| Old release | Semver | Notes |
|-------------|--------|-------|
| v1 | 0.1.1 | First published GitHub release |
| v18-beta | **0.1.8** | Last published release (Sep 2025) |
| v19 (unreleased) | **0.1.9** | Lost local CIA; current `VERSION` |
| v20 (unreleased) | **0.1.10** | Would have been next integer build |

After **0.1.x**, follow normal semver (not `0.1.20` for “v20”):

- **0.2.0** — significant new features (e.g. SMDH fallback, unified title DB merge shipped)
- **1.0.0** — stable, feature-complete enough for a “1.0” homebrew release

## Semver rules (this project)

| Bump | When |
|------|------|
| **MAJOR** | Breaking changes: CIA title ID change (reinstall required), config format break, picker behavior users rely on changes |
| **MINOR** | New features: SMDH lookup, new filters, user-visible functionality in Universal Updater |
| **PATCH** | Bug fixes, title database refresh only, docs/tooling, no user-visible behavior change |

Pre-releases use semver pre-release labels in `VERSION`, e.g. `0.2.0-beta.1`.

## What reads `VERSION`

| Output | Example |
|--------|---------|
| `dist/` filenames | `3DS-Random-Game-Launcher-v0.1.9.3dsx` |
| Debug builds | `3DS-Random-Game-Launcher-v0.1.9-debug.3dsx` |
| Git tag | `v0.1.9` |
| GitHub Release assets | Same basename as `dist/` |
| Universal-DB / Universal Updater | Pulled automatically from GitHub Releases (see [distribution doc](distribution/UNIVERSAL_UPDATER_SETUP.md)) |

The Makefile no longer auto-increments `.build_number` or appends a permanent `-beta` suffix.

## Release checklist

Distribution is via **Universal-DB** (default Universal Updater store). After you publish a GitHub Release with standalone assets, Universal-DB picks up the new version on its own schedule — no separate store file to maintain in this repo.

1. Edit [`VERSION`](../VERSION) per semver rules.
2. Regenerate title database if needed (`scripts/build_title_database.py`), then rebuild.
3. Build: `make` or `build.bat release`; run `build_cia.bat` for the CIA.
4. Verify artifacts in `dist/` (`.3dsx`, `.cia`, optionally `.elf` / `.smdh`).
5. Commit `VERSION` (and any source changes).
6. Tag: `git tag v0.1.9` (match `VERSION`).
7. Push the tag, then **create a GitHub Release** from it on [GitHub](https://github.com/selloa/3DS-Random-Game-Launcher/releases/new):
   - Title and release notes describing changes
   - Upload `dist/*.3dsx` and `dist/*.cia` as **individual files** (not zip-only)
8. Wait for Universal-DB to refresh; confirm the app page shows the new version.

There is **no CI release workflow** in this repo yet — steps 3–7 are manual.

## Development builds

Run `make` or `make DEBUG=1` anytime. Output always reflects the current `VERSION` file — bump `VERSION` only when preparing a release.
