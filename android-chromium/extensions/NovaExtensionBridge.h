#pragma once
#include <string>
#include <vector>

namespace extensions { class ExtensionService; }
class Profile;

namespace nova {

// Native controller owned by the browser's UI/profile lifecycle.
// The caller MUST provide the active Chromium Profile; this avoids guessing
// at Android Profile lifetime from JNI and keeps ExtensionService access on
// the browser UI sequence.
class NovaExtensionBridge {
 public:
  explicit NovaExtensionBridge(Profile* profile);
  ~NovaExtensionBridge();
  NovaExtensionBridge(const NovaExtensionBridge&) = delete;
  NovaExtensionBridge& operator=(const NovaExtensionBridge&) = delete;

  bool Enable(const std::string& id);
  bool Disable(const std::string& id);
  bool Reload(const std::string& id);
  bool Uninstall(const std::string& id);
  bool IsInstalled(const std::string& id) const;
  std::vector<std::string> InstalledIds() const;

  // Installs an already validated unpacked extension directory through the
  // Chromium extension installation pipeline. CRX/ZIP extraction is kept
  // outside this class so the browser can enforce file/permission policy.
  bool InstallUnpacked(const std::string& directory);

 private:
  raw_ptr<Profile> profile_;
  raw_ptr<extensions::ExtensionService> service_;
};

}  // namespace nova
