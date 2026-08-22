package org.chromium.nova.extensions;

import org.chromium.base.annotations.JNINamespace;

@JNINamespace("nova")
public final class NovaExtensionBridgeAndroid {
    static { System.loadLibrary("chrome_public"); }

    private long mNativeBridge;

    private NovaExtensionBridgeAndroid(long nativeBridge) {
        mNativeBridge = nativeBridge;
    }

    public static NovaExtensionBridgeAndroid create(long nativeProfile) {
        long handle = nativeCreate(nativeProfile);
        if (handle == 0) throw new IllegalStateException("Unable to create ExtensionService bridge");
        return new NovaExtensionBridgeAndroid(handle);
    }

    public boolean installCrx(String path) { return mNativeBridge != 0 && nativeInstallCrx(mNativeBridge, path); }
    public boolean installUnpacked(String path) { return mNativeBridge != 0 && nativeInstallUnpacked(mNativeBridge, path); }
    public boolean enable(String id) { return mNativeBridge != 0 && nativeEnable(mNativeBridge, id); }
    public boolean disable(String id) { return mNativeBridge != 0 && nativeDisable(mNativeBridge, id); }
    public boolean reload(String id) { return mNativeBridge != 0 && nativeReload(mNativeBridge, id); }
    public boolean uninstall(String id) { return mNativeBridge != 0 && nativeUninstall(mNativeBridge, id); }

    public void destroy() {
        if (mNativeBridge != 0) {
            nativeDestroy(mNativeBridge);
            mNativeBridge = 0;
        }
    }

    private static native long nativeCreate(long nativeProfile);
    private static native void nativeDestroy(long bridge);
    private static native boolean nativeInstallCrx(long bridge, String path);
    private static native boolean nativeInstallUnpacked(long bridge, String path);
    private static native boolean nativeEnable(long bridge, String id);
    private static native boolean nativeDisable(long bridge, String id);
    private static native boolean nativeReload(long bridge, String id);
    private static native boolean nativeUninstall(long bridge, String id);
}
