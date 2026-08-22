# Chromium integration build notes

The Chromium source is fetched during the build; do not vendor it into AR-CHECK.

## Local Linux build outline

```bash
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git "$HOME/depot_tools"
export PATH="$PATH:$HOME/depot_tools"
mkdir -p "$HOME/chromium" && cd "$HOME/chromium"
fetch --nohooks android
cd src
./build/install-build-deps.sh --android
gclient runhooks
mkdir -p out/Nova
cat > out/Nova/args.gn <<'EOF'
target_os = "android"
target_cpu = "arm64"
is_component_build = false
is_debug = false
EOF
gn gen out/Nova
autoninja -C out/Nova chrome_public_apk
```

Chromium's current Android documentation uses GN/Siso/Ninja and the `chrome_public_apk` target. A full Chromium Android checkout/build is substantially larger than a normal Android Gradle project, so the GitHub workflow is intentionally a bootstrap workflow rather than pretending that a normal `./gradlew assembleDebug` can build Chromium.

## Nova patch set

The next implementation stage must apply a maintained patch set in `chromium-integration/patches/` that adds the Nova UI and the extension import/registration bridge. The patch must be pinned to a specific Chromium revision; do not silently build against an arbitrary moving Chromium revision for releases.
