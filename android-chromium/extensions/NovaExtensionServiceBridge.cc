// Nova Chromium extension bridge.
#include "android-chromium/extensions/NovaExtensionServiceBridge.h"
#include "chrome/browser/extensions/crx_installer.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/unpacked_installer.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"

namespace nova {
NovaExtensionServiceBridge::NovaExtensionServiceBridge(Profile* profile) : profile_(profile) {}
NovaExtensionServiceBridge::~NovaExtensionServiceBridge() = default;

extensions::ExtensionService* NovaExtensionServiceBridge::service() const {
  if (!profile_) return nullptr;
  auto* system = extensions::ExtensionSystem::Get(profile_);
  return system ? system->extension_service() : nullptr;
}

bool NovaExtensionServiceBridge::InstallUnpacked(const base::FilePath& path) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* s = service();
  if (!s || path.empty()) return false;
  extensions::UnpackedInstaller::Create(s)->Load(path);
  return true;
}

bool NovaExtensionServiceBridge::InstallCrx(const base::FilePath& path) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  if (!service() || path.empty()) return false;
  auto installer = extensions::CrxInstaller::CreateSilent(profile_);
  // This is a user-initiated settings-style install, so allow a locally
  // selected CRX instead of treating it as a Web Store download.
  installer->set_off_store_install_allow_reason(
      extensions::CrxInstaller::OffStoreInstallAllowedFromSettingsPage);
  installer->set_allow_silent_install(true);
  installer->set_was_triggered_by_user_download();
  installer->InstallCrx(path);
  return true;
}

bool NovaExtensionServiceBridge::Enable(const std::string& id) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* s = service(); if (!s) return false;
  s->EnableExtension(id); return true;
}

bool NovaExtensionServiceBridge::Disable(const std::string& id) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* s = service(); if (!s) return false;
  s->DisableExtension(id, extensions::disable_reason::DISABLE_USER_ACTION); return true;
}

bool NovaExtensionServiceBridge::Reload(const std::string& id) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* s = service(); if (!s) return false;
  if (!extensions::ExtensionRegistry::Get(profile_)->GetInstalledExtension(id)) return false;
  s->ReloadExtension(id); return true;
}

bool NovaExtensionServiceBridge::Uninstall(const std::string& id) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* s = service(); if (!s) return false;
  if (!extensions::ExtensionRegistry::Get(profile_)->GetInstalledExtension(id)) return false;
  return s->UninstallExtension(id, extensions::UNINSTALL_REASON_USER_INITIATED, nullptr);
}
}  // namespace nova
