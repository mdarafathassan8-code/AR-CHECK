# Nova Browser

Nova Browser is a Google-inspired desktop browser shell built with Electron and Chromium. It is an original interface and is not affiliated with Google.

## Included
- Chromium-powered web browsing
- Tabs, new tab, close tab
- Back, forward, reload and home controls
- Address/search bar
- Unpacked Chrome extension loading
- Windows `.exe` build through GitHub Actions

## Run locally
1. Install Node.js LTS.
2. Run `npm install`.
3. Run `npm start`.

## Build Windows installer
Run `npm run dist`, or push to `main` and use the GitHub Actions workflow. The workflow uploads the generated installer as an artifact.

Electron 43 is used in this project; Electron 43 was released in July 2026 and bundles Chromium 150. citeturn0search0
