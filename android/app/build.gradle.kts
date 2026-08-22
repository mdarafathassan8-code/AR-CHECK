plugins { id("com.android.application"); id("org.jetbrains.kotlin.android") }

android { namespace = "com.nova.browser"; compileSdk = 36
    defaultConfig { applicationId = "com.nova.browser"; minSdk = 26; targetSdk = 36; versionCode = 1; versionName = "1.0.0" }
    compileOptions { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17 }
    kotlinOptions { jvmTarget = "17" }
}

dependencies { implementation("org.mozilla.geckoview:geckoview-stable:156.0.20260820162121") }
