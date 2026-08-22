#ifndef NOVA_ANDROID_CHROMIUM_EXTENSIONS_NOVA_EXTENSION_SERVICE_BRIDGE_H_
#define NOVA_ANDROID_CHROMIUM_EXTENSIONS_NOVA_EXTENSION_SERVICE_BRIDGE_H_

#include <string>
#include "base/files/file_path.h"
#include "base/memory/raw_ptr.h"

class Profile;
namespace extensions { class ExtensionService; }

namespace nova {
class NovaExtensionServiceBridge {
 public:
  explicit NovaExtensionServiceBridge(Profile* profile);
  ~NovaExtensionServiceBridge();
  NovaExtensionServiceBridge(const NovaExtensionServiceBridge&) = delete;
  NovaExtensionServiceBridge& operator=(const NovaExtensionServiceBridge&) = delete;

  bool InstallUnpacked(const base::FilePath& path);
  bool InstallCrx(const base::FilePath& path);
  bool Enable(const std::string& id);
  bool Disable(const std::string& id);
  bool Reload(const std::string& id);
  bool Uninstall(const std::string& id);

 private:
  extensions::ExtensionService* service() const;
  raw_ptr<Profile> profile_ = nullptr;
};
}  // namespace nova
#endif
