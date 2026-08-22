// Copyright 2026 Nova Browser contributors
// SPDX-License-Identifier: BSD-3-Clause
#ifndef CHROME_BROWSER_EXTENSIONS_NOVA_EXTENSION_MANAGER_ANDROID_H_
#define CHROME_BROWSER_EXTENSIONS_NOVA_EXTENSION_MANAGER_ANDROID_H_

#include <string>
#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"

class Profile;

namespace extensions {

// Android-facing bridge for installing and managing user extensions.
// Supported input types:
//   .crx       -> CrxInstaller
//   directory  -> UnpackedInstaller
//   .zip       -> extracted to a private temporary directory, then
//                 UnpackedInstaller
//
// All methods must be called on the UI thread. The bridge intentionally uses
// Chromium's existing ExtensionService/ExtensionRegistry instead of creating
// a second extension runtime.
class NovaExtensionManagerAndroid {
 public:
  using ResultCallback = base::OnceCallback<void(bool success,
                                                  std::string message)>;

  explicit NovaExtensionManagerAndroid(Profile* profile);
  ~NovaExtensionManagerAndroid();

  NovaExtensionManagerAndroid(const NovaExtensionManagerAndroid&) = delete;
  NovaExtensionManagerAndroid& operator=(const NovaExtensionManagerAndroid&) = delete;

  void Install(const base::FilePath& source, ResultCallback callback);
  void Uninstall(const std::string& extension_id, ResultCallback callback);
  void Enable(const std::string& extension_id, ResultCallback callback);
  void Disable(const std::string& extension_id, ResultCallback callback);
  void Reload(const std::string& extension_id, ResultCallback callback);

  bool IsInstalled(const std::string& extension_id) const;
  bool IsEnabled(const std::string& extension_id) const;

 private:
  void InstallZip(const base::FilePath& zip, ResultCallback callback);
  void InstallCrx(const base::FilePath& crx, ResultCallback callback);
  void InstallUnpacked(const base::FilePath& directory,
                       ResultCallback callback);

  void Finish(ResultCallback callback, bool success, std::string message);

  raw_ptr<Profile> profile_ = nullptr;
  base::WeakPtrFactory<NovaExtensionManagerAndroid> weak_factory_{this};
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_NOVA_EXTENSION_MANAGER_ANDROID_H_
