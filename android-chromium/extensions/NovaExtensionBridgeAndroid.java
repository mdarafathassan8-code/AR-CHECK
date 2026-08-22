package org.chromium.nova.extensions;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.NativeMethods;

@JNINamespace("nova")
public final class NovaExtensionBridgeAndroid {
    private long mNativeBridge;

    public NovaExtensionBridgeAndroid(long nativeBridge) {
        mNativeBridge = nativeBridge;
    }

    public boolean enable(String id) { return NovaExtensionBridgeAndroidJni.get().enable(mNativeBridge, id); }
    public boolean disable(String id) { return NovaExtensionBridgeAndroidJni.get().disable(mNativeBridge, id); }
    public boolean reload(String id) { return NovaExtensionBridgeAndroidJni.get().reload(mNativeBridge, id); }
    public boolean uninstall(String id) { return NovaExtensionBridgeAndroidJni.get().uninstall(mNativeBridge, id); }
    public String[] installedIds() { return NovaExtensionBridgeAndroidJni.get().installedIds(mNativeBridge); }

    @CalledByNative
    public static void onNativeBridgeDestroyed(NovaExtensionBridgeAndroid bridge) {
        bridge.mNativeBridge = 0;
    }

    @NativeMethods
    interface N {
        boolean enable(long nativeBridge, String id);
        boolean disable(long nativeBridge, String id);
        boolean reload(long nativeBridge, String id);
        boolean uninstall(long nativeBridge, String id);
        String[] installedIds(long nativeBridge);
    }

    private static class NovaExtensionBridgeAndroidJni {
        static N get() { return org.chromium.nova.extensions.NovaExtensionBridgeAndroidJni.get(); }
    }
}
