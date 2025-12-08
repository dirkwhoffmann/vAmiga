#ifndef LINUXGUI_SERVICES_CONFIG_PROVIDER_H_
#define LINUXGUI_SERVICES_CONFIG_PROVIDER_H_

#include <filesystem>
#include <string>
#include <string_view>

#include "VAmiga.h"

namespace gui {

struct ConfigKeys {
  static constexpr std::string_view kKickstartPath = "KickstartPath";
  static constexpr std::string_view kExtRomPath    = "ExtRomPath";

  // Input
  static constexpr std::string_view kPauseBg       = "Input.PauseInBackground";
  static constexpr std::string_view kRetainClick   = "Input.RetainMouseByClick";
  static constexpr std::string_view kRetainEnter   = "Input.RetainMouseByEntering";
  static constexpr std::string_view kShakeRelease  = "Input.ReleaseMouseByShaking";

  // Audio
  static constexpr std::string_view kAudioVolume   = "Audio.Volume";
  static constexpr std::string_view kAudioSep      = "Audio.Separation";
};

class ConfigProvider {
 public:
  explicit ConfigProvider(vamiga::DefaultsAPI& defaults_api);

  void Load();
  void Save();

  std::string GetString(std::string_view key, const std::string& fallback = "");
  void SetString(std::string_view key, const std::string& value);

  bool GetBool(std::string_view key, bool fallback = false);
  void SetBool(std::string_view key, bool value);

  int GetInt(std::string_view key, int fallback = 0);
  void SetInt(std::string_view key, int value);

  std::string GetFloppyPath(int drive);
  void SetFloppyPath(int drive, const std::string& path);

 private:
  std::filesystem::path GetConfigPath() const;

  vamiga::DefaultsAPI& defaults_;
};

}  // namespace gui

#endif  // LINUXGUI_SERVICES_CONFIG_PROVIDER_H_
