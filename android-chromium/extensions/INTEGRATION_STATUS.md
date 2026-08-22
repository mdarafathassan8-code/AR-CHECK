# Nova Chromium Extension Integration

This directory contains the native bridge contract and Android manager shell.

## Required final wiring

The exact Chromium revision used by the build MUST be pinned before compiling the bridge. `Profile` and `ExtensionService` APIs are internal Chromium APIs and can change between revisions.

The intended runtime is:

Android file picker -> private staging -> CRX/ZIP/unpacked validation -> permission UI -> Chromium installer -> active Profile -> ExtensionService.

The native bridge currently implements the stable lifecycle operations against `ExtensionService`: enable, disable, reload, uninstall and registry listing.

`InstallUnpacked()` and the Android native-handle creation are intentionally not guessed. They must be connected to the pinned revision's `CrxInstaller`/`UnpackedInstaller` API and the browser's real Profile lifecycle. Returning a fake success here would produce an APK that appears to install extensions but does not actually load them.

## Chromium build

Official Chromium Android builds use `chrome_public_apk`; the exact command is documented by Chromium. Use a Linux x86-64 runner with the required Chromium checkout and dependencies.
