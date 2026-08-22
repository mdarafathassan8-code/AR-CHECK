#include "android-chromium/extensions/NovaExtensionBridge.h"

#include "base/functional/bind.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/extension.h"

namespace nova {
NovaExtensionBridge::NovaExtensionBridge(Profile* profile) : profile_(profile) {
  CHECK(profile_);
  service_ = extensions::ExtensionSystem::Get(profile_)->extension_service();
  CHECK(service_);
}
NovaExtensionBridge::~NovaExtensionBridge() = default;

bool NovaExtensionBridge::Enable(const std::string& id) {
  if (!service_ || !extensions::ExtensionRegistry::Get(profile_)->GetExtensionById(id)) return false;
  service_->EnableExtension(id);
  return true;
}
bool NovaExtensionBridge::Disable(const std::string& id) {
  if (!service_ || !extensions::ExtensionRegistry::Get(profile_)->GetExtensionById(id)) return false;
  service_->DisableExtension(id, extensions::disable_reason::DISABLE_USER_ACTION);
  return true;
}
bool NovaExtensionBridge::Reload(const std::string& id) {
  if (!service_ || !extensions::ExtensionRegistry::Get(profile_)->GetExtensionById(id)) return false;
  service_->ReloadExtension(id, base::DoNothing());
  return true;
}
bool NovaExtensionBridge::Uninstall(const std::string& id) {
  if (!service_ || !extensions::ExtensionRegistry::Get(profile_)->GetExtensionById(id)) return false;
  service_->UninstallExtension(id, extensions::UNINSTALL_REASON_USER_INITIATED, base::DoNothing());
  return true;
}
bool NovaExtensionBridge::IsInstalled(const std::string& id) const {
  auto* registry = extensions::ExtensionRegistry::Get(profile_);
  return registry && registry->GetExtensionById(id) != nullptr;
}
std::vector<std::string> NovaExtensionBridge::InstalledIds() const {
  std::vector<std::string> ids;
  auto* registry = extensions::ExtensionRegistry::Get(profile_);
  if (!registry) return ids;
  for (const auto& extension : registry->enabled_extensions()) ids.push_back(extension->id());
  for (const auto& extension : registry->disabled_extensions()) ids.push_back(extension->id());
  return ids;
}
bool NovaExtensionBridge::InstallUnpacked(const std::string& directory) {
  // Deliberately not implemented by calling an internal installer with a
  // guessed signature. Wire this method to the CrxInstaller/UnpackedInstaller
  // API for the exact Chromium revision pinned by the build manifest.
  return false;
}
}  // namespace nova
