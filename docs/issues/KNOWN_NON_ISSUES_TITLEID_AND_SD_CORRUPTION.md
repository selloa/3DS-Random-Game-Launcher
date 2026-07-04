# Known Non-Issues: TitleID vs SD Corruption Artifacts

This document captures common reports that can look like app bugs, but are usually unrelated to launcher code.

## Scope

- Project: `3DS-Random-Game-Launcher`
- Topic: title ID category confusion (`00040000` vs `0004000e`) and broken SD title layout
- Goal: avoid misdiagnosing filesystem corruption as an app packaging issue

## Current App Category (Expected)

From `tools/3DS-Random-Game-Launcher.rsf`:

- `TitleInfo.Category : Application`
- `TitleInfo.UniqueId : 0x5247C`

Expected title high category for the base app is therefore:

- `00040000` (application/base title family)

## Why `0004000e` May Still Appear

`0004000e` is the update/patch title high category on 3DS.

If support tools or bots mention `0004000e` with this app name, that does **not** automatically mean this project was built as an update title. In practice, this often indicates broken title DB/folder mapping on SD (or interrupted installs/updates), for example:

- title DB entries out of sync with actual folders
- leftover folders from interrupted installs/uninstalls
- SD corruption creating malformed or orphaned directory entries
- partial/manual backup restore side effects

## About Empty `content/00000001` Reports

Community tooling has reported at least one case of this app name appearing with an empty `content/00000001` folder under a title path where only `00000000` is normally expected.

Interpretation:

- usually a storage/layout artifact, not app logic
- likely a stale or malformed content index folder
- consistent with corruption/incomplete write scenarios

This is best treated as a title storage consistency issue, not as proof that launcher source code selected the wrong title family.

## Practical Guidance

- Treat `0004000e` mentions from repair scripts as diagnostics context first.
- Confirm build configuration in RSF before assuming packaging bugs.
- Prefer SD/title database cleanup workflows (backup first) when symptoms include:
  - missing HOME Menu titles
  - wrong names/icons
  - question mark/black box entries
  - data management mismatch vs HOME Menu visibility

## Non-Issue Summary

For this repository, reports that mention `0004000e` association or empty secondary content folders are currently tracked as **likely environmental/storage corruption artifacts**, not confirmed launcher code defects.
