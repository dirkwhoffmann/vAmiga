#ifndef LINUXGUI_APPLICATION_H_
#define LINUXGUI_APPLICATION_H_

#include <SDL.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "VAmiga.h"
#include "components/input_manager.h"
#include "services/config_provider.h"

struct SDLWindowDeleter {
  void operator()(SDL_Window* w) const {
    if (w) SDL_DestroyWindow(w);
  }
};

using SDLWindowPtr = std::unique_ptr<SDL_Window, SDLWindowDeleter>;
using SDLGLContextPtr = std::unique_ptr<void, void (*)(void*)>;

class Application {
 public:
  Application(int argc, char** argv);
  ~Application();

  bool Init();
  void Run();

  void LoadKickstart(const std::filesystem::path& path);
  void EjectKickstart();
  void LoadExtendedRom(const std::filesystem::path& path);
  void EjectExtendedRom();
  void InsertFloppy(int drive, const std::filesystem::path& path);
  void EjectFloppy(int drive);
  void TogglePower();
  void HardReset();
  void ToggleRunPause();

  void LoadSnapshot(const std::filesystem::path& path);
  void SaveSnapshot(const std::filesystem::path& path);

  vamiga::VAmiga& GetEmulator() { return emulator_; }
  SDL_Window* GetWindow() { return window_.get(); }

 private:
  bool InitSDL();
  void InitImGui();
  void InitEmulator();
  void LoadConfig();
  void SaveConfig();

  void MainLoop();
  void HandleEvents(bool& done);
  void Update();
  void Render();
  void DrawGUI();

  SDLWindowPtr window_;
  SDLGLContextPtr gl_context_;

  vamiga::VAmiga emulator_;
  std::unique_ptr<InputManager> input_manager_;
  std::unique_ptr<gui::ConfigProvider> config_;

  bool show_settings_ = false;
  bool show_inspector_ = false;
  bool show_dashboard_ = false;
  bool show_console_ = false;
  bool show_keyboard_ = false;

  std::string kickstart_path_;
  std::string ext_rom_path_;
  std::string floppy_paths_[4];

  int chip_ram_idx_ = 1;
  int slow_ram_idx_ = 0;
  int fast_ram_mb_ = 0;
  int agnus_rev_ = 0;
  int denise_rev_ = 0;
  int rtc_model_ = 0;
  int volume_ = 0;
  int separation_ = 100;
  int current_standard_ = 0;
  int scale_factor_ = 1;
  int filter_mode_ = 1;
  int port1_device_ = 1;
  int port2_device_ = 2;
};

#endif  // LINUXGUI_APPLICATION_H_