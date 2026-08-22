package com.nova.browser

import android.app.*
import android.content.*
import android.net.Uri
import android.os.*
import android.widget.*
import org.mozilla.geckoview.*
import java.io.File

class MainActivity : Activity() {
    companion object { private var runtime: GeckoRuntime? = null }
    private lateinit var session: GeckoSession
    private lateinit var view: GeckoView
    private lateinit var address: EditText
    private lateinit var controller: WebExtensionController
    private val pickExtension = 7001

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        view = findViewById(R.id.geckoview); address = findViewById(R.id.address)
        val settings = GeckoRuntimeSettings.Builder()
            .extensionsProcessEnabled(true)
            .extensionsWebAPIEnabled(true)
            .build()
        if (runtime == null) runtime = GeckoRuntime.create(this, settings)
        session = GeckoSession(); session.open(runtime!!); view.setSession(session)
        controller = runtime!!.webExtensionController
        session.loadUri("https://www.google.com")

        findViewById<Button>(R.id.go).setOnClickListener { navigate(address.text.toString()) }
        address.setOnEditorActionListener { _, _, _ -> navigate(address.text.toString()); true }
        findViewById<Button>(R.id.back).setOnClickListener { session.goBack() }
        findViewById<Button>(R.id.forward).setOnClickListener { session.goForward() }
        findViewById<Button>(R.id.menu).setOnClickListener { showMenu() }
    }

    private fun navigate(value: String) {
        val v = value.trim(); if (v.isEmpty()) return
        val url = if (v.startsWith("http://") || v.startsWith("https://")) v
        else if (v.contains(".") && !v.contains(" ")) "https://$v"
        else "https://www.google.com/search?q=" + Uri.encode(v)
        session.loadUri(url)
    }

    private fun showMenu() {
        AlertDialog.Builder(this).setTitle("Nova Browser")
            .setItems(arrayOf("🧩 Install Extension", "🧩 Extension Manager", "⭐ Bookmarks", "🕘 History", "📥 Downloads", "🕶️ Private tab")) { _, which ->
                when (which) { 0 -> pickExtension(); 1 -> extensionManager(); 5 -> session.loadUri("https://www.google.com") }
            }.show()
    }

    private fun pickExtension() {
        val i = Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
            type = "application/x-xpinstall"; addCategory(Intent.CATEGORY_OPENABLE); putExtra(Intent.EXTRA_MIME_TYPES, arrayOf("application/x-xpinstall", "application/zip", "application/octet-stream"))
        }
        startActivityForResult(i, pickExtension)
    }

    @Deprecated("Use Activity Result API in a future cleanup")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode != pickExtension || resultCode != RESULT_OK || data?.data == null) return
        installExtension(data.data!!)
    }

    private fun installExtension(uri: Uri) {
        try {
            val input = contentResolver.openInputStream(uri) ?: throw Exception("Cannot read selected file")
            val file = File(cacheDir, "extension-${System.currentTimeMillis()}.xpi")
            input.use { it.copyTo(file.outputStream()) }
            controller.install(Uri.fromFile(file).toString(), WebExtensionController.INSTALLATION_METHOD_FROM_FILE)
                .accept({ ext -> runOnUiThread { Toast.makeText(this, "Installed: ${ext.metaData.name}", Toast.LENGTH_LONG).show() } }, { e -> runOnUiThread { Toast.makeText(this, "Extension install failed: ${e.message}", Toast.LENGTH_LONG).show() } })
        } catch (e: Exception) { Toast.makeText(this, "Extension error: ${e.message}", Toast.LENGTH_LONG).show() }
    }

    private fun extensionManager() {
        controller.list().accept({ list ->
            runOnUiThread {
                if (list.isEmpty()) { Toast.makeText(this, "No extensions installed", Toast.LENGTH_SHORT).show(); return@runOnUiThread }
                val names = list.map { it.metaData.name ?: it.id }.toTypedArray()
                AlertDialog.Builder(this).setTitle("Extensions").setItems(names) { _, index ->
                    val ext = list[index]
                    AlertDialog.Builder(this).setTitle(ext.metaData.name ?: ext.id)
                        .setItems(arrayOf("Enable", "Disable", "Remove")) { _, action ->
                            when (action) {
                                0 -> controller.enable(ext, WebExtensionController.EnableSource.USER)
                                1 -> controller.disable(ext, WebExtensionController.EnableSource.USER)
                                2 -> controller.uninstall(ext)
                            }
                        }.show()
                }.show()
            }
        }, {})
    }
}
