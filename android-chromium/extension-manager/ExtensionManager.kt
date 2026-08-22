package com.nova.browser.extensions

import android.content.Context
import android.net.Uri
import java.io.File

/** Mobile-facing contract for the native Chromium extension bridge.
 *  The actual extension runtime remains Chromium's ExtensionService.
 */
class ExtensionManager(private val context: Context, private val bridge: ChromiumExtensionBridge) {
    fun import(uri: Uri): Result<ExtensionInfo> = runCatching {
        val name = uri.lastPathSegment ?: error("Invalid extension file")
        when {
            name.endsWith(".crx", true) -> bridge.installCrx(uri)
            name.endsWith(".zip", true) -> bridge.installZip(uri)
            else -> error("Choose a .crx, .zip, or unpacked extension directory")
        }
    }

    fun installUnpacked(directory: File): Result<ExtensionInfo> = runCatching {
        require(directory.isDirectory) { "Not a directory" }
        require(File(directory, "manifest.json").isFile) { "manifest.json not found" }
        bridge.installUnpacked(directory)
    }

    fun enable(id: String) = bridge.setEnabled(id, true)
    fun disable(id: String) = bridge.setEnabled(id, false)
    fun reload(id: String) = bridge.reload(id)
    fun remove(id: String) = bridge.remove(id)
    fun installed() = bridge.list()
}

data class ExtensionInfo(val id: String, val name: String, val version: String, val enabled: Boolean, val error: String? = null)

interface ChromiumExtensionBridge {
    fun installCrx(uri: Uri): ExtensionInfo
    fun installZip(uri: Uri): ExtensionInfo
    fun installUnpacked(directory: File): ExtensionInfo
    fun setEnabled(id: String, enabled: Boolean)
    fun reload(id: String)
    fun remove(id: String)
    fun list(): List<ExtensionInfo>
}
