# Nova Browser Android — detailed structure

```text
android-chromium/
├── README.md
├── ARCHITECTURE.md
├── app/
│   ├── src/main/AndroidManifest.xml
│   ├── src/main/java/com/nova/browser/
│   │   ├── MainActivity.kt
│   │   ├── BrowserApplication.kt
│   │   ├── BrowserController.kt
│   │   ├── Tab.kt
│   │   └── DownloadController.kt
│   └── src/main/res/
│       ├── layout/
│       ├── drawable/
│       ├── mipmap-anydpi-v26/
│       └── values/
├── browser/
│   ├── TabManager.kt
│   ├── NavigationController.kt
│   ├── PrivateSessionManager.kt
│   └── BrowserPreferences.kt
├── extensions/
│   ├── ExtensionManager.kt
│   ├── ExtensionPackage.kt
│   ├── ExtensionManifest.kt
│   ├── CrxReader.kt
│   ├── ZipExtensionReader.kt
│   ├── UnpackedExtensionReader.kt
│   ├── ExtensionPermissionPolicy.kt
│   └── ExtensionStore.kt
├── tests/
│   ├── extension/
│   └── browser/
└── chromium-integration/
    ├── PATCHES.md
    └── BUILD_NOTES.md
```

## Runtime flow

`MainActivity` -> `BrowserController` -> Chromium browser/content layer.

Extension import:

`file picker` -> `ExtensionPackageDetector` -> CRX/ZIP/unpacked reader -> `manifest.json` validation -> permission review -> persistent extension record -> Chromium extension registration.

## Security rules

1. Never execute an imported package before manifest validation.
2. Extract ZIP/CRX into an app-private directory.
3. Reject path traversal (`../`) and absolute archive paths.
4. Show requested permissions before enabling an extension.
5. Keep private-session extensions/data isolated from normal browsing data.
6. Do not grant native Android permissions merely because an extension requests a similarly named web permission.
7. Keep the Chromium sandbox/security model enabled; do not weaken it to make an extension work.

## Compatibility

The target is Chromium/WebExtension compatibility, not a promise that every Chrome extension works. Unsupported Chrome APIs must fail safely or be surfaced by the extension manager.

## Build strategy

The repository contains the Nova integration code and CI scripts, not the multi-gigabyte Chromium source tree. CI checks out Chromium with `depot_tools`, applies Nova patches, generates an Android build with GN, and builds the APK with Ninja. This follows the official Chromium Android build workflow.
