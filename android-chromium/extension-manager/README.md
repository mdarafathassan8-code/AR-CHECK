# Nova Android Extension Manager

This module is the UI/IPC contract for the Chromium Android extension manager.

It is designed to sit above Chromium's existing extension service rather than reimplementing the extension runtime. Chromium already provides the core extension system and an Android extensions WebUI; Nova adds a mobile-friendly manager and import workflow.

Supported import inputs:
- `.crx`: pass the file to Chromium's CRX installer after validation.
- `.zip`: extract to an app-private staging directory, validate `manifest.json`, then install as an unpacked/developer extension where the Chromium build permits it.
- Unpacked directory: validate `manifest.json`, then load through Chromium's unpacked installer.

The manager must never execute archive contents during import. ZIP extraction must prevent path traversal and must keep staging data inside the app's private directory.

UI states: Installed, Enabled, Disabled, Error. Actions: Install, Enable, Disable, Reload, Remove, Open options.
