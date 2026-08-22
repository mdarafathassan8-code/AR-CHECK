#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/containers/span.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/browser_thread.h"
#include "android-chromium/extensions/NovaExtensionBridge.h"
#include "android-chromium/extensions/jni_headers/NovaExtensionBridgeAndroid_jni.h"

namespace nova {

static NovaExtensionBridge* FromHandle(jlong handle) {
  return reinterpret_cast<NovaExtensionBridge*>(handle);
}

jboolean JNI_NovaExtensionBridgeAndroid_Enable(JNIEnv* env, jlong handle, const JavaParamRef<jstring>& jid) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* bridge = FromHandle(handle);
  return bridge && bridge->Enable(base::android::ConvertJavaStringToUTF8(env, jid));
}

jboolean JNI_NovaExtensionBridgeAndroid_Disable(JNIEnv* env, jlong handle, const JavaParamRef<jstring>& jid) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* bridge = FromHandle(handle);
  return bridge && bridge->Disable(base::android::ConvertJavaStringToUTF8(env, jid));
}

jboolean JNI_NovaExtensionBridgeAndroid_Reload(JNIEnv* env, jlong handle, const JavaParamRef<jstring>& jid) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* bridge = FromHandle(handle);
  return bridge && bridge->Reload(base::android::ConvertJavaStringToUTF8(env, jid));
}

jboolean JNI_NovaExtensionBridgeAndroid_Uninstall(JNIEnv* env, jlong handle, const JavaParamRef<jstring>& jid) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* bridge = FromHandle(handle);
  return bridge && bridge->Uninstall(base::android::ConvertJavaStringToUTF8(env, jid));
}

base::android::ScopedJavaLocalRef<jobjectArray> JNI_NovaExtensionBridgeAndroid_InstalledIds(JNIEnv* env, jlong handle) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* bridge = FromHandle(handle);
  std::vector<std::string> ids = bridge ? bridge->InstalledIds() : std::vector<std::string>();
  auto result = base::android::ToJavaArrayOfStrings(env, ids);
  return result;
}

}  // namespace nova
