# Nova Browser — Chromium Android Architecture

This directory defines the Android architecture for Nova Browser. It is intentionally separate from the existing Electron and GitHub Pages prototypes.

## Goal

Build an Android browser from Chromium source with a native Nova UI and an extension import pipeline for:

- unpacked extension directories containing `manifest.json`
- `.zip` packages containing `manifest.json`
- `.crx` packages (extract/validate the CRX payload before installation)

## Important engineering constraint

This project does **not** pretend that Android WebView can run arbitrary Chrome extensions. The extension-capable browser must be built from/against Chromium browser code. The official Chromium Android build requires a Linux x86-64 build host and a large checkout; the current Chromium documentation recommends at least 8 GB RAM, more than 16 GB recommended, and about 100 GB free disk space.

## Layers

1. `app/` — Nova Android UI and browser chrome.
2. `browser/` — tab/session/navigation controller.
3. `extensions/` — package detection, validation, extraction, manifest parsing, permission display, lifecycle state.
4. `chromium/` — integration boundary for the Chromium checkout/patch set; Chromium itself is fetched by CI rather than copied into this repository.
5. `tests/` — extension package and browser-controller tests.

## Extension import policy

- ZIP: only accept a ZIP whose extracted root contains a valid `manifest.json` (or a single top-level directory containing it).
- Unpacked: select a directory and validate `manifest.json` before enabling it.
- CRX: parse CRX2/CRX3, extract the embedded ZIP payload, then pass the resulting extension package through the same validator. The Chromium integration layer must enforce the actual extension runtime/security rules.

A successful import means the package passed validation; it does **not** guarantee every Chrome API is supported by the chosen Chromium revision.
