// Nova Chromium extension bridge.
// This file intentionally uses Chromium's current Profile/ExtensionService APIs.
#include "android-chromium/extensions/NovaExtensionServiceBridge.h"

#include "base/files/file_path.h"
#include "base/memory/scoped_refptr.h"
#include "chrome/browser/extensions/crx_installer.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/unpacked_installer.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/extension.h"

namespace nova {

NovaExtensionServiceBridge::NovaExtensionServiceBridge(Profile* profile)
    : profile_(profile) {}
NovaExtensionServiceBridge::~NovaExtensionServiceBridge() = default;

extensions::ExtensionService* NovaExtensionServiceBridge::service() const {
  return profile_ ? extensions::ExtensionSystem::Get(profile_)->extension_service()
                  : nullptr;
}

bool NovaExtensionServiceBridge::InstallUnpacked(const base::FilePath& path) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* s = service();
  if (!s || path.empty()) return false;
  auto installer = extensions::UnpackedInstaller::Create(s);
  installer->Load(path);
  return true;
}

bool NovaExtensionServiceBridge::InstallCrx(const base::FilePath& path) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  auto* s = service();
  if (!s || path.empty()) return false;
  // CRX installation is asynchronous. The installer owns the unpack/validation
  // pipeline and reports completion through its normal callbacks.
  auto installer = base::MakeRefCounted<extensions::CrxInstaller>(
      s->AsWeakPtr(), nullptr, nullptr);
  installer->InstallCrx(path);
  return true;
}

bool NovaExtensionServiceBridge::Enable(const std::string& id) {
  auto* s = service();
  if (!s) return false;
  s->EnableExtension(id);
  return true;
}

bool NovaExtensionServiceBridge::Disable(const std::string& id) {
  auto* s = service();
  if (!s) return false;
  s->DisableExtension(id, extensions::disable_reason::DISABLE_USER_ACTION);
  return true;
}

bool NovaExtensionServiceBridge::Uninstall(const std::string& id) {
  auto* s = service();
  if (!s) return false;
  const auto* ext = extensions::ExtensionRegistry::Get(profile_)->GetInstalledExtension(id);
  if (!ext) return false;
  s->UninstallExtension(id, extensions::UNINSTALL_REASON_USER_INITIATED, nullptr);
  return true;
}

}  // namespace nova
