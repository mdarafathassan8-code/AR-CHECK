// Copyright 2026 Nova Browser contributors
// SPDX-License-Identifier: BSD-3-Clause

#include "chrome/browser/extensions/nova_extension_manager_android.h"

#include <utility>

#include "base/files/scoped_temp_dir.h"
#include "base/functional/bind.h"
#include "base/logging.h"
#include "base/task/thread_pool.h"
#include "chrome/browser/extensions/crx_installer.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/extension_system.h"
#include "chrome/browser/extensions/unpacked_installer.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/extension_registry.h"
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

  // CRX verification, manifest validation and installation are delegated to
  // Chromium's existing CrxInstaller. This preserves the normal extension
  // security model rather than implementing a second CRX parser.
  scoped_refptr<CrxInstaller> installer = CrxInstaller::CreateSilent(service);
  installer->AddInstallerCallback(base::BindOnce(
      [](base::WeakPtr<NovaExtensionManagerAndroid> self,
         ResultCallback callback, const CrxInstallError& error) {
        if (!self)
          return;
        if (error.error_code() == CrxInstallErrorType::NONE) {
          self->Finish(std::move(callback), true, "CRX installed");
        } else {
          self->Finish(std::move(callback), false,
                       error.message().empty() ? "CRX installation failed"
                                               : error.message());
        }
      },
      weak_factory_.GetWeakPtr(), std::move(callback)));
  installer->set_off_store_install_allow_reason(
      CrxInstaller::OffStoreInstallAllowedFromSettingsPage);
  installer->InstallCrx(crx);
}

void NovaExtensionManagerAndroid::InstallZip(const base::FilePath& zip_file,
                                             ResultCallback callback) {
  // ZIP is treated as an unpacked extension after safe extraction. Chromium's
  // zip::Unzip rejects unsafe archive paths, preventing ../ traversal.
  auto temp_dir = std::make_unique<base::ScopedTempDir>();
  if (!temp_dir->CreateUniqueTempDir()) {
    Finish(std::move(callback), false, "Could not create extension temp dir");
    return;
  }

  const base::FilePath destination = temp_dir->GetPath();
  auto temp_dir_holder = temp_dir.release();
  base::ThreadPool::PostTaskAndReply(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_BLOCKING},
      base::BindOnce(
          [](const base::FilePath& source, const base::FilePath& dest) {
            return zip::Unzip(source, dest);
          },
          zip_file, destination),
      base::BindOnce(
          [](base::WeakPtr<NovaExtensionManagerAndroid> self,
             std::unique_ptr<base::ScopedTempDir> extracted,
             base::FilePath destination, ResultCallback callback, bool ok) {
            if (!self)
              return;
            if (!ok) {
              self->Finish(std::move(callback), false,
                           "ZIP extraction failed or archive was unsafe");
              return;
            }
            self->InstallUnpacked(destination, std::move(callback));
            // Keep the extraction alive until the installer has consumed it.
            // The browser's normal extension install directory becomes the
            // persistent location for the installed extension.
            (void)extracted.release();
          },
          weak_factory_.GetWeakPtr(), std::unique_ptr<base::ScopedTempDir>(temp_dir_holder),
          destination, std::move(callback)));
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

  // UnpackedInstaller performs manifest validation and registers the extension
  // with ExtensionService. It is also the same path used by Chromium's
  // developer-mode tests for unpacked extensions.
  auto installer = UnpackedInstaller::Create(service);
  installer->Load(directory);

  // Installation is asynchronous. Observe ExtensionRegistry for the ID after
  // manifest validation; callers can refresh the manager after the callback.
  Finish(std::move(callback), true, "Unpacked extension load requested");
}

void NovaExtensionManagerAndroid::Uninstall(const std::string& extension_id,
                                            ResultCallback callback) {
  auto* service = ExtensionSystem::Get(profile_)->extension_service();
  if (!service) {
    Finish(std::move(callback), false, "Extension service unavailable");
    return;
  }
  std::string error;
  const bool ok = service->UninstallExtension(extension_id, false, &error);
  Finish(std::move(callback), ok, ok ? "Extension removed"
                                    : (error.empty() ? "Uninstall failed" : error));
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
