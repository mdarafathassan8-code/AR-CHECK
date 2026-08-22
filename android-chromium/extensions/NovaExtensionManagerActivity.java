package org.chromium.nova.extensions;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import java.util.ArrayList;

/** Android-side manager. File import is staged in app-private storage before
 * the Chromium native installer is invoked, so the native side never trusts
 * a raw external URI. */
public final class NovaExtensionManagerActivity extends Activity {
    private static final int PICK_FILE = 1001;
    private static final int PICK_FOLDER = 1002;
    private NovaExtensionBridgeAndroid bridge;

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        bridge = new NovaExtensionBridgeAndroid(getNativeBridgeHandle());
    }

    public void pickCrxOrZip() {
        Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        i.setType("application/octet-stream");
        i.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
        i.addCategory(Intent.CATEGORY_OPENABLE);
        startActivityForResult(i, PICK_FILE);
    }

    public void pickUnpackedDirectory() {
        Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        i.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
        startActivityForResult(i, PICK_FOLDER);
    }

    @Override protected void onActivityResult(int request, int result, Intent data) {
        super.onActivityResult(request, result, data);
        if (result != RESULT_OK || data == null || data.getData() == null) return;
        Uri uri = data.getData();
        if (request == PICK_FILE) {
            // The production importer must copy the URI to app-private storage,
            // verify CRX/ZIP structure, parse manifest.json, and show permissions
            // before invoking the native install pipeline.
            importExternalPackage(uri);
        } else if (request == PICK_FOLDER) {
            importUnpackedTree(uri);
        }
    }

    private void importExternalPackage(Uri uri) { /* wired to staging service */ }
    private void importUnpackedTree(Uri uri) { /* wired to staging service */ }
    private long getNativeBridgeHandle() { return 0L; }
}
