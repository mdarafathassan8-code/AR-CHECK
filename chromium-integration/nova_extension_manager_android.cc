// Copyright 2026 Nova Browser contributors
// SPDX-License-Identifier: BSD-3-Clause

#include "chrome/browser/extensions/nova_extension_manager_android.h"

#include <optional>
#include <utility>

#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/thread_pool.h"
#include "content/public/browser/browser_thread.h"
#include "chrome/browser/extensions/crx_installer.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/extension_system.h"
#include "chrome/browser/extensions/unpacked_installer.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/install/crx_install_error.h"
#include "extensions/common/extension.h"
#include "third_party/zlib/google/zip.h"

namespace extensions {

NovaExtensionManagerAndroid::NovaExtensionManagerAndroid(Profile* profile)
    : profile_(profile) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  CHECK(profile_);
}

NovaExtensionManagerAndroid::~NovaExtensionManagerAndroid() = default;

void NovaExtensionManagerAndroid::Install(const base::FilePath& source,
                                           ResultCallback callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  if (source.empty() || !base::PathExists(source)) {
    Finish(std::move(callback), false, "Extension source does not exist");
    return;
  }

  const std::string extension = source.Extension();
  if (extension == ".crx") {
    InstallCrx(source, std::move(callback));
    return;
  }
  if (extension == ".zip") {
    InstallZip(source, std::move(callback));
    return;
  }
  if (base::DirectoryExists(source)) {
    InstallUnpacked(source, std::move(callback));
    return;
  }

  Finish(std::move(callback), false,
         "Select a .crx, .zip, or unpacked extension directory");
}

void NovaExtensionManagerAndroid::InstallCrx(const base::FilePath& crx,
                                             ResultCallback callback) {
  auto* system = ExtensionSystem::Get(profile_);
  auto* service = system ? system->extension_service() : nullptr;
  if (!service || !service->extensions_enabled()) {
    Finish(std::move(callback), false, "Chromium extension service is disabled");
    return;
  }

  // CRX3 signature verification, manifest validation, policy checks and the
  // actual registration are delegated to Chromium's existing installer.
  scoped_refptr<CrxInstaller> installer = CrxInstaller::CreateSilent(service);
  installer->set_allow_silent_install(true);
  installer->set_off_store_install_allow_reason(
      CrxInstaller::OffStoreInstallAllowedFromSettingsPage);
  installer->AddInstallerCallback(base::BindOnce(
      [](base::WeakPtr<NovaExtensionManagerAndroid> self,
         ResultCallback callback,
         const std::optional<CrxInstallError>& error) {
        if (!self)
          return;
        if (!error.has_value()) {
          self->Finish(std::move(callback), true, "CRX installed");
          return;
        }
        const std::string message =
            base::UTF16ToUTF8(error->message()).empty()
                ? "CRX installation failed"
                : base::UTF16ToUTF8(error->message());
        self->Finish(std::move(callback), false, message);
      },
      weak_factory_.GetWeakPtr(), std::move(callback)));
  installer->InstallCrx(crx);
}

void NovaExtensionManagerAndroid::InstallZip(const base::FilePath& zip_file,
                                             ResultCallback callback) {
  // ZIP is treated as an unpacked extension after extraction. Chromium's
  // zip::Unzip rejects unsafe paths such as ../ traversal entries.
  auto temp_dir = std::make_unique<base::ScopedTempDir>();
  if (!temp_dir->CreateUniqueTempDir()) {
    Finish(std::move(callback), false, "Could not create extension temp dir");
    return;
  }

  const base::FilePath destination = temp_dir->GetPath();
  auto* temp_dir_raw = temp_dir.get();
  zip_temp_dirs_.push_back(std::move(temp_dir));

  base::ThreadPool::PostTaskAndReply(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_BLOCKING},
      base::BindOnce(
          [](const base::FilePath& source, const base::FilePath& dest) {
            return zip::Unzip(source, dest);
          },
          zip_file, destination),
      base::BindOnce(
          [](base::WeakPtr<NovaExtensionManagerAndroid> self,
             base::FilePath destination, ResultCallback callback, bool ok) {
            if (!self)
              return;
            if (!ok) {
              self->Finish(std::move(callback), false,
                           "ZIP extraction failed or archive was unsafe");
              return;
            }
            self->InstallUnpacked(destination, std::move(callback));
          },
          weak_factory_.GetWeakPtr(), destination, std::move(callback)));
  (void)temp_dir_raw;
}

void NovaExtensionManagerAndroid::InstallUnpacked(
    const base::FilePath& directory,
    ResultCallback callback) {
  auto* system = ExtensionSystem::Get(profile_);
  auto* service = system ? system->extension_service() : nullptr;
  if (!service || !service->extensions_enabled()) {
    Finish(std::move(callback), false, "Chromium extension service is disabled");
    return;
  }

  // This is Chromium's developer-mode installer path. It validates
  // manifest.json and registers the unpacked extension with ExtensionService.
  auto installer = UnpackedInstaller::Create(service);
  installer->Load(directory);

  // UnpackedInstaller::Load() is asynchronous and has no simple result
  // callback. The UI should refresh its extension list after the load event.
  Finish(std::move(callback), true, "Unpacked extension load requested");
}

void NovaExtensionManagerAndroid::Uninstall(const std::string& extension_id,
                                            ResultCallback callback) {
  auto* service = ExtensionSystem::Get(profile_)->extension_service();
  if (!service) {
    Finish(std::move(callback), false, "Extension service unavailable");
    return;
  }
  std::u16string error;
  const bool ok = service->UninstallExtension(extension_id, false, &error);
  Finish(std::move(callback), ok,
         ok ? "Extension removed" : base::UTF16ToUTF8(error));
}

void NovaExtensionManagerAndroid::Enable(const std::string& extension_id,
                                         ResultCallback callback) {
  auto* service = ExtensionSystem::Get(profile_)->extension_service();
  if (!service) {
    Finish(std::move(callback), false, "Extension service unavailable");
    return;
  }
  service->EnableExtension(extension_id);
  Finish(std::move(callback), true, "Extension enabled");
}

void NovaExtensionManagerAndroid::Disable(const std::string& extension_id,
                                          ResultCallback callback) {
  auto* service = ExtensionSystem::Get(profile_)->extension_service();
  if (!service) {
    Finish(std::move(callback), false, "Extension service unavailable");
    return;
  }
  service->DisableExtension(extension_id);
  Finish(std::move(callback), true, "Extension disabled");
}

void NovaExtensionManagerAndroid::Reload(const std::string& extension_id,
                                         ResultCallback callback) {
  auto* service = ExtensionSystem::Get(profile_)->extension_service();
  if (!service) {
    Finish(std::move(callback), false, "Extension service unavailable");
    return;
  }
  service->ReloadExtension(extension_id);
  Finish(std::move(callback), true, "Extension reloaded");
}

bool NovaExtensionManagerAndroid::IsInstalled(
    const std::string& extension_id) const {
  auto* registry = ExtensionRegistry::Get(profile_);
  return registry && registry->GetInstalledExtension(extension_id) != nullptr;
}

bool NovaExtensionManagerAndroid::IsEnabled(
    const std::string& extension_id) const {
  auto* registry = ExtensionRegistry::Get(profile_);
  return registry && registry->enabled_extensions().Contains(extension_id);
}

void NovaExtensionManagerAndroid::Finish(ResultCallback callback,
                                         bool success,
                                         std::string message) {
  if (callback)
    std::move(callback).Run(success, std::move(message));
}

}  // namespace extensions
