package com.nova.browser.extensions

import android.app.Activity
import android.os.Bundle
import android.widget.LinearLayout
import android.widget.TextView
import android.widget.Button

/** Lightweight mobile manager screen. Wire the ChromiumExtensionBridge in the app layer. */
class ExtensionManagerActivity : Activity() {
    override fun onCreate(state: Bundle?) {
        super.onCreate(state)
        val root = LinearLayout(this).apply { orientation = LinearLayout.VERTICAL; setPadding(24, 24, 24, 24) }
        root.addView(TextView(this).apply { text = "Nova Extensions"; textSize = 28f })
        root.addView(TextView(this).apply { text = "Install .crx, .zip or an unpacked extension.\n\nOnly trusted extensions should be installed."; textSize = 15f; setPadding(0, 18, 0, 18) })
        root.addView(Button(this).apply { text = "Install Extension" })
        root.addView(Button(this).apply { text = "Refresh" })
        setContentView(root)
    }
}
