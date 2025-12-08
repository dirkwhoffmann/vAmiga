#include "config_provider.h"

#include <charconv>
#include <cstdlib>
#include <format>
#include <iostream>

namespace gui {

ConfigProvider::ConfigProvider(vamiga::DefaultsAPI& defaults_api)
    : defaults_(defaults_api) {}

std::filesystem::path ConfigProvider::GetConfigPath() const {
  const char* home = getenv("HOME");
  std::filesystem::path config_dir = home ? std::filesystem::path(home) / ".config" / "vamiga" : ".";
  std::error_code ec;
  if (!std::filesystem::exists(config_dir, ec)) {
    std::filesystem::create_directories(config_dir, ec);
  }
  return config_dir / "vamiga.config";
}

void ConfigProvider::Load() {
  try {
    std::filesystem::path path = GetConfigPath();
    if (std::filesystem::exists(path)) {
      defaults_.load(path);
    }
  } catch (const std::exception& e) {
    std::cerr << "Config Load Error: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Config Load Error: Unknown" << std::endl;
  }
}

void ConfigProvider::Save() {
  try {
    defaults_.save(GetConfigPath());
  } catch (const std::exception& e) {
    std::cerr << "Config Save Error: " << e.what() << std::endl;
  } catch (...) {
  }
}

std::string ConfigProvider::GetString(std::string_view key,
                                      const std::string& fallback) {
  try {
    std::string val = defaults_.getRaw(std::string(key));
    return val.empty() ? fallback : val;
  } catch (...) {
    return fallback;
  }
}

void ConfigProvider::SetString(std::string_view key, const std::string& value) {
  try {
    defaults_.set(std::string(key), value);
  } catch (...) {
  }
}

bool ConfigProvider::GetBool(std::string_view key, bool fallback) {
  std::string val = GetString(key, "");
  if (val == "1") return true;
  if (val == "0") return false;
  return fallback;
}

void ConfigProvider::SetBool(std::string_view key, bool value) {
  SetString(key, value ? "1" : "0");
}

int ConfigProvider::GetInt(std::string_view key, int fallback) {
  std::string val = GetString(key, "");
  if (val.empty()) return fallback;

  int result = 0;
  auto [ptr, ec] =
      std::from_chars(val.data(), val.data() + val.size(), result);
  if (ec == std::errc()) {
    return result;
  }
  return fallback;
}

void ConfigProvider::SetInt(std::string_view key, int value) {
  SetString(key, std::to_string(value));
}

std::string ConfigProvider::GetFloppyPath(int drive) {
  return GetString(std::format("DF{}Path", drive));
}

void ConfigProvider::SetFloppyPath(int drive, const std::string& path) {
  SetString(std::format("DF{}Path", drive), path);
}

}  // namespace gui
