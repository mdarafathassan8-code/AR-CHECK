#include <jni.h>

#include "android-chromium/extensions/NovaExtensionServiceBridge.h"
#include "base/android/jni_android.h"
#include "chrome/browser/profiles/profile.h"

namespace {
using Bridge = nova::NovaExtensionServiceBridge;
Bridge* FromHandle(jlong handle) {
  return reinterpret_cast<Bridge*>(handle);
}
}

extern "C" JNIEXPORT jlong JNICALL
Java_org_chromium_nova_extensions_NovaExtensionBridgeAndroid_nativeCreate(
    JNIEnv*, jclass, jlong profile_ptr) {
  auto* profile = reinterpret_cast<Profile*>(profile_ptr);
  if (!profile) return 0;
  return reinterpret_cast<jlong>(new Bridge(profile));
}

extern "C" JNIEXPORT void JNICALL
Java_org_chromium_nova_extensions_NovaExtensionBridgeAndroid_nativeDestroy(
    JNIEnv*, jclass, jlong bridge_ptr) {
  delete FromHandle(bridge_ptr);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_chromium_nova_extensions_NovaExtensionBridgeAndroid_nativeInstallCrx(
    JNIEnv* env, jclass, jlong bridge_ptr, jstring path) {
  const char* chars = env->GetStringUTFChars(path, nullptr);
  bool ok = FromHandle(bridge_ptr)->InstallCrx(base::FilePath::FromUTF8Unsafe(chars));
  env->ReleaseStringUTFChars(path, chars);
  return ok;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_chromium_nova_extensions_NovaExtensionBridgeAndroid_nativeInstallUnpacked(
    JNIEnv* env, jclass, jlong bridge_ptr, jstring path) {
  const char* chars = env->GetStringUTFChars(path, nullptr);
  bool ok = FromHandle(bridge_ptr)->InstallUnpacked(base::FilePath::FromUTF8Unsafe(chars));
  env->ReleaseStringUTFChars(path, chars);
  return ok;
}

#define NOVA_JNI_ACTION(Name, Method) \
extern "C" JNIEXPORT jboolean JNICALL \
Java_org_chromium_nova_extensions_NovaExtensionBridgeAndroid_native##Name( \
    JNIEnv* env, jclass, jlong bridge_ptr, jstring id) { \
  const char* chars = env->GetStringUTFChars(id, nullptr); \
  bool ok = FromHandle(bridge_ptr)->Method(chars); \
  env->ReleaseStringUTFChars(id, chars); \
  return ok; \
}

NOVA_JNI_ACTION(Enable, Enable)
NOVA_JNI_ACTION(Disable, Disable)
NOVA_JNI_ACTION(Reload, Reload)
NOVA_JNI_ACTION(Uninstall, Uninstall)

#undef NOVA_JNI_ACTION
