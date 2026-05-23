# Distribution (Universal Updater / Universal-DB)

This app is distributed through **[Universal-DB](https://db.universal-team.net/)**, the default app store built into [Universal Updater](https://github.com/Universal-Team/Universal-Updater). Users do not need a custom UniStore or QR code.

## How users install and update

1. Open **Universal Updater** on a 3DS with homebrew access.
2. Make sure **Universal-DB** is selected (it is the default store).
3. Search for **"3DS Random Game Launcher"**.
4. Install the **CIA** (homescreen icon) or **3DSX** (Homebrew Launcher) from the app page.
5. When a new GitHub Release is published, Universal Updater shows an update badge after Universal-DB refreshes (typically within a few hours).

Direct download from [GitHub Releases](https://github.com/selloa/3DS-Random-Game-Launcher/releases) remains available for users who prefer manual installs.

## Listing in Universal-DB

The app is registered in [Universal-Team/db](https://github.com/Universal-Team/db) (`source/apps/3ds-random-game-laucher.json`), pointing at `selloa/3DS-Random-Game-Launcher`.

**Maintaining the Universal-DB listing:** edit [`3ds-random-game-laucher.json`](3ds-random-game-laucher.json) in this repo, then open a PR on [Universal-Team/db](https://github.com/Universal-Team/db) replacing `source/apps/3ds-random-game-laucher.json` with that file (filename typo `laucher` is intentional — match the existing entry).

**First-time inclusion** is a one-time manual step: submit via the [App Request form](https://db.universal-team.net/app-request) and open a PR or issue on Universal-DB. That step is already done for this project.

**Metadata** (title, description, icon, banner) comes from the app JSON in Universal-DB and your GitHub repo. To change long description or icons, update the Universal-DB entry or the URLs it references (`meta/banner.png`, `icon.png` on `main`).

## How updates reach Universal Updater

There is no push API. Universal-DB runs scheduled GitHub Actions that:

- Read each listed app's GitHub repo and **releases**
- Generate install scripts for standalone `.3dsx` / `.cia` assets
- Rebuild `universal-db.unistore`, which Universal Updater downloads

**Your release responsibility:** publish a semver GitHub Release with individual `.3dsx` and `.cia` files attached. Do **not** rely on zip-only releases — Universal-DB handles standalone binaries automatically; archives need extra configuration.

See [VERSIONING.md](../VERSIONING.md) for the full release checklist.

## Release assets Universal-DB expects

Upload these from `dist/` when creating a GitHub Release (tag `vX.Y.Z`, matching [`VERSION`](../../VERSION)):

| File | Example |
|------|---------|
| 3DSX | `3DS-Random-Game-Launcher-v0.1.9.3dsx` |
| CIA | `3DS-Random-Game-Launcher-v0.1.9.cia` |

Build locally:

```bash
make                    # or build.bat release
build_cia.bat           # Windows — adds .cia to dist/
```

## Assets used by Universal-DB

These files in the repo are referenced by the Universal-DB listing (full map: [../meta/README.md](../meta/README.md)):

| Path | Purpose |
|------|---------|
| `icon.png` | App icon in Universal Updater |
| `meta/banner.png` | Banner on the app page |
| `meta/screenshot.png` | Optional screenshot |

Keep them on `main` at stable raw GitHub URLs.

## Troubleshooting

**App not showing an update in Universal Updater**

- Confirm a GitHub Release exists for the new tag and assets are uploaded.
- Universal-DB may take up to ~6 hours to refresh (hourly for priority apps).
- Check [Universal-DB app page](https://db.universal-team.net/3ds/3ds-random-game-launcher) for the version it currently advertises.

**Download or install fails**

- Ensure release assets are standalone `.3dsx` / `.cia`, not only a zip.
- CIA installs require a CFW environment; 3DSX requires Homebrew Launcher.

**Need to change Universal-DB listing**

- Open an issue or PR on [Universal-Team/db](https://github.com/Universal-Team/db), or ask on the [Universal-Team Discord](https://universal-team.net/discord).
