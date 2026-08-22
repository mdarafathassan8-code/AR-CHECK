package com.nova.browser.extensions

import android.content.Context
import android.net.Uri

/** Thin Kotlin facade over the Chromium-native extension service bridge. */
class NovaExtensionBridge(private val context: Context) {
    companion object { init { System.loadLibrary("nova_extensions") } }

    external fun nativeInstall(path: String, type: Int): String
    external fun nativeSetEnabled(extensionId: String, enabled: Boolean): Boolean
    external fun nativeReload(extensionId: String): Boolean
    external fun nativeUninstall(extensionId: String): Boolean
    external fun nativeList(): String

    fun install(uri: Uri, type: ImportType): String =
        nativeInstall(uri.toString(), type.code)

    enum class ImportType(val code: Int) { CRX(1), ZIP(2), UNPACKED(3) }
}
