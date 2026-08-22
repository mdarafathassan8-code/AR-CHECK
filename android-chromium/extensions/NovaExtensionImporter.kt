package org.chromium.nova.extensions

import android.content.Context
import android.net.Uri
import java.io.File
import java.util.zip.ZipInputStream
import org.json.JSONObject

/** Copies user-selected CRX/ZIP/unpacked data into app-private storage before native install. */
class NovaExtensionImporter(private val context: Context) {
    private val root = File(context.filesDir, "nova_extensions")

    fun importPackage(uri: Uri): Result = runCatching {
        root.mkdirs()
        val name = "ext_${System.currentTimeMillis()}"
        val input = context.contentResolver.openInputStream(uri) ?: error("Cannot open file")
        val raw = File(root, "$name.input")
        input.use { src -> raw.outputStream().use { dst -> src.copyTo(dst) } }
        val lower = (context.contentResolver.getType(uri) ?: "").lowercase()
        when {
            lower.contains("zip") || raw.name.endsWith(".zip", true) ->
                Result.unpackZip(raw, File(root, name))
            else -> {
                val crx = File(root, "$name.crx")
                raw.renameTo(crx)
                Result.Crx(crx)
            }
        }
    }.getOrElse { Result.Error(it.message ?: "Import failed") }

    fun importUnpacked(treeRoot: File): Result = runCatching {
        validateManifest(treeRoot)
        Result.Unpacked(treeRoot)
    }.getOrElse { Result.Error(it.message ?: "Invalid extension") }

    private fun validateManifest(dir: File) {
        val manifest = File(dir, "manifest.json")
        require(manifest.isFile) { "manifest.json not found" }
        val json = JSONObject(manifest.readText())
        require(json.optInt("manifest_version", 0) in 2..3) { "Unsupported manifest_version" }
        require(json.optString("name").isNotBlank()) { "Extension name missing" }
    }

    sealed class Result {
        data class Crx(val file: File): Result()
        data class Unpacked(val dir: File): Result()
        data class Error(val message: String): Result()
        companion object {
            fun unpackZip(zip: File, destination: File): Result {
                destination.mkdirs()
                ZipInputStream(zip.inputStream().buffered()).use { zis ->
                    while (true) {
                        val e = zis.nextEntry ?: break
                        val target = File(destination, e.name)
                        require(target.canonicalPath.startsWith(destination.canonicalPath + File.separator)) { "Unsafe ZIP entry" }
                        if (e.isDirectory) target.mkdirs() else {
                            target.parentFile?.mkdirs()
                            target.outputStream().use { out -> zis.copyTo(out) }
                        }
                    }
                }
                val candidates = listOf(destination, destination.listFiles()?.singleOrNull { it.isDirectory } ?: destination)
                val dir = candidates.firstOrNull { File(it, "manifest.json").isFile } ?: error("manifest.json not found in ZIP")
                val manifest = File(dir, "manifest.json")
                val json = JSONObject(manifest.readText())
                require(json.optInt("manifest_version", 0) in 2..3) { "Unsupported manifest_version" }
                return Unpacked(dir)
            }
        }
    }
}
