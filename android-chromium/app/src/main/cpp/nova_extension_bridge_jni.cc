#include <jni.h>
#include <string>
#include "base/android/jni_android.h"
#include "base/android/scoped_java_ref.h"

// Integration point for the browser Profile's ExtensionService.
// The Profile/ExtensionService lookup must happen on the browser UI sequence.
namespace {
std::string JStringToString(JNIEnv* env, jstring value) {
  if (!value) return {};
  const char* chars = env->GetStringUTFChars(value, nullptr);
  std::string result = chars ? chars : "";
  if (chars) env->ReleaseStringUTFChars(value, chars);
  return result;
}

jstring StringToJString(JNIEnv* env, const std::string& value) {
  return env->NewStringUTF(value.c_str());
}
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_nova_browser_extensions_NovaExtensionBridge_nativeInstall(
    JNIEnv* env, jobject, jstring uri, jint type) {
  // TODO: resolve the active Profile and call the NovaExtensionManager C++
  // service. `type` is CRX=1, ZIP=2, UNPACKED=3. This JNI boundary is kept
  // deliberately small so all Chromium APIs remain on the native side.
  return StringToJString(env, "NOT_WIRED_TO_PROFILE_YET");
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_nova_browser_extensions_NovaExtensionBridge_nativeSetEnabled(
    JNIEnv*, jobject, jstring extension_id, jboolean enabled) {
  return JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_nova_browser_extensions_NovaExtensionBridge_nativeReload(
    JNIEnv*, jobject, jstring extension_id) {
  return JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_nova_browser_extensions_NovaExtensionBridge_nativeUninstall(
    JNIEnv*, jobject, jstring extension_id) {
  return JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_nova_browser_extensions_NovaExtensionBridge_nativeList(
    JNIEnv* env, jobject) {
  return StringToJString(env, "[]");
}
