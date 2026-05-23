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
| **MINOR** | New features: SMDH lookup, new filters, UniStore-visible functionality |
| **PATCH** | Bug fixes, title database refresh only, docs/tooling, no user-visible behavior change |

Pre-releases use semver pre-release labels in `VERSION`, e.g. `0.2.0-beta.1`.

## What reads `VERSION`

| Output | Example |
|--------|---------|
| `dist/` filenames | `3DS-Random-Game-Launcher-v0.1.9.3dsx` |
| Debug builds | `3DS-Random-Game-Launcher-v0.1.9-debug.3dsx` |
| Git tag | `v0.1.9` |
| GitHub Release assets | Same basename as `dist/` |
| [`unistore.json`](../unistore.json) | `"version": "0.1.9"` (must match manually on release) |

The Makefile no longer auto-increments `.build_number` or appends a permanent `-beta` suffix.

## Release checklist

1. Edit [`VERSION`](../VERSION) per semver rules.
2. Update [`unistore.json`](../unistore.json): `apps[].version`, `releaseNotes`, and `downloadUrl` if needed.
3. Build: `make` or `build.bat release`.
4. Verify artifacts in `dist/`.
5. Commit `VERSION` (and `unistore.json` if changed).
6. Tag: `git tag v$(cat VERSION)` (e.g. `v0.1.9`).
7. Create GitHub Release from the tag; upload `dist/*` assets.
8. For CIA builds, bump makerom version fields to match (see [tools/README.md](../tools/README.md)).

## Development builds

Run `make` or `make DEBUG=1` anytime. Output always reflects the current `VERSION` file — bump `VERSION` only when preparing a release.
