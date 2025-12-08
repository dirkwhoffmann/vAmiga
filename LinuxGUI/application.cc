#include "application.h"

#include <SDL_opengl.h>

#include <format>
#include <iostream>

#include "Ports/AudioPort.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "components/console.h"
#include "components/dashboard.h"
#include "components/file_picker.h"
#include "components/inspector.h"
#include "components/virtual_keyboard.h"
#include "core_actions.h"
#include "imgui.h"

namespace ImGui {
bool InputText(const char* label, std::string* str,
               ImGuiInputTextFlags flags = 0) {
  return InputText(
      label, str->data(), str->capacity() + 1,
      flags | ImGuiInputTextFlags_CallbackResize,
      [](ImGuiInputTextCallbackData* data) -> int {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
          std::string* s = (std::string*)data->UserData;
          s->resize(data->BufTextLen);
          data->Buf = s->data();
        }
        return 0;
      },
      str);
}
}  // namespace ImGui

Application::Application(int argc, char** argv)
    : gl_context_(nullptr, SDL_GL_DeleteContext) {}

Application::~Application() {
  SaveConfig();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_Quit();
}

bool Application::Init() {
  if (!InitSDL()) return false;
  InitImGui();
  InitEmulator();
  config_ = std::make_unique<gui::ConfigProvider>(emulator_.defaults);
  LoadConfig();
  return true;
}

bool Application::InitSDL() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER |
               SDL_INIT_AUDIO) != 0) {
    std::cerr << std::format("SDL_Init Error: {}", SDL_GetError()) << std::endl;
    return false;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  window_.reset(SDL_CreateWindow(
      "vAmiga Linux", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI));

  if (!window_) {
    std::cerr << std::format("SDL_CreateWindow Error: {}", SDL_GetError())
              << std::endl;
    return false;
  }

  gl_context_.reset(SDL_GL_CreateContext(window_.get()));
  if (!gl_context_) {
    std::cerr << std::format("SDL_GL_CreateContext Error: {}", SDL_GetError())
              << std::endl;
    return false;
  }
  SDL_GL_MakeCurrent(window_.get(), gl_context_.get());
  SDL_GL_SetSwapInterval(1);
  return true;
}

void Application::InitImGui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForOpenGL(window_.get(), gl_context_.get());
  ImGui_ImplOpenGL3_Init("#version 130");
}

void Application::InitEmulator() {
  emulator_.set(vamiga::ConfigScheme::A500_OCS_1MB);
  emulator_.set(vamiga::Opt::AMIGA_VSYNC, 0);
  emulator_.set(vamiga::Opt::AUD_ASR, 1);
  emulator_.set(vamiga::Opt::AUD_SAMPLING_METHOD,
                (vamiga::i64)vamiga::SamplingMethod::LINEAR);
  emulator_.set(vamiga::Opt::AUD_BUFFER_SIZE, 16384);

  emulator_.defaults.setFallback("KickstartPath", "");
  emulator_.defaults.setFallback("ExtRomPath", "");
  emulator_.defaults.setFallback("DF0Path", "");
  emulator_.defaults.setFallback("DF1Path", "");
  emulator_.defaults.setFallback("DF2Path", "");
  emulator_.defaults.setFallback("DF3Path", "");

  emulator_.defaults.setFallback("Input.PauseInBackground", "1");
  emulator_.defaults.setFallback("Input.RetainMouseByClick", "1");
  emulator_.defaults.setFallback("Input.RetainMouseByEntering", "0");
  emulator_.defaults.setFallback("Input.ReleaseMouseByShaking", "1");
  emulator_.defaults.setFallback("Audio.Volume", "100");
  emulator_.defaults.setFallback("Audio.Separation", "100");

  input_manager_ = std::make_unique<InputManager>(emulator_);
  emulator_.launch();

  SDL_AudioSpec want{}, have{};
  want.freq = 44100;
  want.format = AUDIO_F32;
  want.channels = 2;
  want.samples = 1024;
  want.callback = [](void* userdata, Uint8* stream, int len) {
    auto* emu = (vamiga::VAmiga*)userdata;
    int num_frames = len / (sizeof(float) * 2);
    int copied = emu->audioPort.copyInterleaved((float*)stream, num_frames);
    if (copied < num_frames) {
      memset(stream + (copied * sizeof(float) * 2), 0,
             (num_frames - copied) * sizeof(float) * 2);
    }
  };
  want.userdata = &emulator_;

  SDL_AudioDeviceID dev = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
  if (dev) {
    emulator_.audioPort.port->setSampleRate(have.freq);
    SDL_PauseAudioDevice(dev, 0);
  }

  gui::Console::Instance().SetCommandCallback([this](const std::string& cmd) {
    emulator_.retroShell.press(cmd);
    emulator_.retroShell.press(vamiga::RSKey::RETURN, false);
  });
}

void Application::LoadConfig() {
  config_->Load();

  input_manager_->pause_in_background_ =
      config_->GetBool(gui::ConfigKeys::kPauseBg, true);
  input_manager_->retain_mouse_by_click_ =
      config_->GetBool(gui::ConfigKeys::kRetainClick, true);
  input_manager_->retain_mouse_by_entering_ =
      config_->GetBool(gui::ConfigKeys::kRetainEnter, false);
  input_manager_->release_mouse_by_shaking_ =
      config_->GetBool(gui::ConfigKeys::kShakeRelease, true);

  kickstart_path_ = config_->GetString(gui::ConfigKeys::kKickstartPath);
  ext_rom_path_ = config_->GetString(gui::ConfigKeys::kExtRomPath);
  for (int i = 0; i < 4; ++i)
    floppy_paths_[i] = config_->GetFloppyPath(i);

  if (!kickstart_path_.empty()) LoadKickstart(kickstart_path_);
  if (!ext_rom_path_.empty()) LoadExtendedRom(ext_rom_path_);
  for (int i = 0; i < 4; ++i)
    if (!floppy_paths_[i].empty()) InsertFloppy(i, floppy_paths_[i]);

  agnus_rev_ = (int)emulator_.get(vamiga::Opt::AGNUS_REVISION);
  denise_rev_ = (int)emulator_.get(vamiga::Opt::DENISE_REVISION);
  rtc_model_ = (int)emulator_.get(vamiga::Opt::RTC_MODEL);
  volume_ = (int)emulator_.get(vamiga::Opt::AUD_VOLL);
  current_standard_ = (int)emulator_.get(vamiga::Opt::AMIGA_VIDEO_FORMAT);
}

void Application::SaveConfig() {
  config_->SetBool(gui::ConfigKeys::kPauseBg,
                   input_manager_->pause_in_background_);
  config_->SetBool(gui::ConfigKeys::kRetainClick,
                   input_manager_->retain_mouse_by_click_);
  config_->SetBool(gui::ConfigKeys::kRetainEnter,
                   input_manager_->retain_mouse_by_entering_);
  config_->SetBool(gui::ConfigKeys::kShakeRelease,
                   input_manager_->release_mouse_by_shaking_);

  config_->Save();
}

void Application::Run() {
  if (!Init()) return;
  MainLoop();
}

void Application::MainLoop() {
  bool done = false;
  while (!done) {
    input_manager_->SetPortDevices(port1_device_, port2_device_);
    input_manager_->Update();

    HandleEvents(done);
    Update();
    Render();
    emulator_.wakeUp();
  }
}

void Application::HandleEvents(bool& done) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT) done = true;
    if (event.type == SDL_WINDOWEVENT) {
      if (event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(window_.get()))
        done = true;
      if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
        input_manager_->HandleWindowFocus(true);
      if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        input_manager_->HandleWindowFocus(false);
    }
    if (event.type == SDL_DROPFILE) {
      std::filesystem::path path(event.drop.file);
      std::string ext = path.extension().string();
      for (auto& c : ext) c = tolower(c);

      if (ext == ".adf" || ext == ".adz" || ext == ".dms" || ext == ".ipf")
        InsertFloppy(0, path);
      else if (ext == ".rom" || ext == ".bin")
        LoadKickstart(path);
      else if (ext == ".vsn")
        LoadSnapshot(path);

      SDL_free(event.drop.file);
    }
    input_manager_->HandleEvent(event);
  }
}

void Application::Update() {}

void Application::Render() {
  static GLuint video_texture = 0;
  if (video_texture == 0) {
    glGenTextures(1, &video_texture);
    glBindTexture(GL_TEXTURE_2D, video_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glBindTexture(GL_TEXTURE_2D, video_texture);
  GLint filter = (filter_mode_ == 0) ? GL_NEAREST : GL_LINEAR;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

  emulator_.videoPort.lockTexture();
  const uint32_t* pixels = emulator_.videoPort.getTexture();
  if (pixels) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vamiga::HPIXELS, vamiga::VPIXELS, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  }
  emulator_.videoPort.unlockTexture();

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  DrawGUI();

  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x,
                                 viewport->Pos.y + ImGui::GetFrameHeight() + 40));
  ImGui::SetNextWindowSize(ImVec2(
      viewport->Size.x, viewport->Size.y - ImGui::GetFrameHeight() - 40));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::Begin("Screen", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);

  input_manager_->SetViewportHovered(ImGui::IsWindowHovered());

  ImVec2 avail = ImGui::GetContentRegionAvail();
  float aspect = (float)vamiga::HPIXELS / (float)vamiga::VPIXELS;
  ImVec2 sz = avail;
  if (sz.x / sz.y > aspect)
    sz.x = sz.y * aspect;
  else
    sz.y = sz.x / aspect;
  sz.x *= scale_factor_;
  sz.y *= scale_factor_;

  if (sz.x < avail.x) ImGui::SetCursorPosX((avail.x - sz.x) * 0.5f);
  if (sz.y < avail.y) ImGui::SetCursorPosY((avail.y - sz.y) * 0.5f);

  ImGui::Image((void*)(intptr_t)video_texture, sz);
  ImGui::End();
  ImGui::PopStyleVar();

  ImGui::Render();
  ImGuiIO& io = ImGui::GetIO();
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(window_.get());
}

void DrawRomInfo(const char* label, const vamiga::RomTraits& traits,
                 bool present, std::string* path_buffer,
                 std::function<void()> on_load, std::function<void()> on_eject) {
  ImGui::PushID(label);
  ImGui::BeginGroup();
  ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", label);
  ImGui::Separator();
  ImGui::Spacing();
  ImGui::BeginGroup();
  ImGui::ColorButton("Icon",
                     present ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)
                             : ImVec4(0.8f, 0.2f, 0.2f, 1.0f),
                     ImGuiColorEditFlags_NoTooltip, ImVec2(48, 48));
  ImGui::SameLine();
  ImGui::BeginGroup();
  if (present) {
    ImGui::Text("Title:    %s", traits.title ? traits.title : "Unknown");
    ImGui::Text("Revision: %s", traits.revision ? traits.revision : "-");
    ImGui::Text("Released: %s", traits.released ? traits.released : "-");
    ImGui::Text("Model:    %s", traits.model ? traits.model : "-");
    ImGui::TextDisabled("CRC32:    0x%08X", traits.crc);
  } else {
    ImGui::Text("No ROM loaded");
    ImGui::TextDisabled("Drag/Drop or Load");
  }
  ImGui::EndGroup();
  ImGui::EndGroup();
  ImGui::Spacing();
  ImGui::Text("Path:");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(-120);
  ImGui::InputText("##path", path_buffer);
  ImGui::SameLine();
  if (ImGui::Button("Load", ImVec2(50, 0))) {
    gui::PickerOptions opts;
    opts.title = std::string("Select ") + label;
    opts.filters =
        "ROM Files (*.rom *.bin){.rom,.bin,.adf},All Files (*.*){.*}";
    gui::FilePicker::Instance().Open(
        std::string("RomPicker_") + label, opts,
        [path_buffer, on_load](std::filesystem::path p) {
          *path_buffer = p.string();
          on_load();
        });
  }
  ImGui::SameLine();
  if (ImGui::Button("Eject", ImVec2(50, 0))) on_eject();
  ImGui::EndGroup();
  ImGui::PopID();
}

void Application::DrawGUI() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open Snapshot")) {
        gui::PickerOptions opts;
        opts.title = "Open Snapshot";
        opts.filters = "Snapshot Files (*.vsn){.vsn}";
        gui::FilePicker::Instance().Open("SnapLoad", opts, [this](auto p) {
          LoadSnapshot(p);
        });
      }
      if (ImGui::MenuItem("Save Snapshot")) {
        gui::PickerOptions opts;
        opts.title = "Save Snapshot";
        opts.mode = gui::PickerMode::kSaveFile;
        opts.filters = "Snapshot Files (*.vsn){.vsn}";
        gui::FilePicker::Instance().Open("SnapSave", opts, [this](auto p) {
          SaveSnapshot(p);
        });
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Quit", "Alt+F4")) {
        SDL_Event q;
        q.type = SDL_QUIT;
        SDL_PushEvent(&q);
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Tools")) {
      ImGui::MenuItem("Settings", nullptr, &show_settings_);
      if (ImGui::BeginMenu("Inspectors")) {
        ImGui::MenuItem("Inspector", nullptr, &show_inspector_);
        ImGui::MenuItem("Dashboard", nullptr, &show_dashboard_);
        ImGui::MenuItem("Console", nullptr, &show_console_);
        ImGui::MenuItem("Virtual Keyboard", nullptr, &show_keyboard_);
        ImGui::EndMenu();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(
      ImVec2(viewport->Pos.x, viewport->Pos.y + ImGui::GetFrameHeight()));
  ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 40));
  ImGui::Begin("Toolbar", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);

  if (ImGui::Button("Settings")) {
    show_settings_ = !show_settings_;
  }
  ImGui::SameLine();

  ImGui::BeginGroup();
  if (ImGui::Button("Insp")) {
    show_inspector_ = !show_inspector_;
  }
  ImGui::SameLine();
  if (ImGui::Button("Dash")) {
    show_dashboard_ = !show_dashboard_;
  }
  ImGui::SameLine();
  if (ImGui::Button("Cons")) {
    show_console_ = !show_console_;
  }
  ImGui::EndGroup();
  ImGui::SameLine();

  ImGui::BeginGroup();
  if (ImGui::Button("Save")) {
    gui::PickerOptions opts;
    opts.title = "Save Snapshot";
    opts.mode = gui::PickerMode::kSaveFile;
    opts.filters = "Snapshot Files (*.vsn){.vsn}";
    gui::FilePicker::Instance().Open("SnapSaveBtn", opts, [this](auto p) {
      SaveSnapshot(p);
    });
  }
  ImGui::SameLine();
  if (ImGui::Button("Load")) {
    gui::PickerOptions opts;
    opts.title = "Open Snapshot";
    opts.filters = "Snapshot Files (*.vsn){.vsn}";
    gui::FilePicker::Instance().Open("SnapLoadBtn", opts, [this](auto p) {
      LoadSnapshot(p);
    });
  }
  ImGui::EndGroup();
  ImGui::SameLine();

  ImGui::SetNextItemWidth(100);
  const char* devices[] = {"None",      "Mouse",     "Keyset 1",
                           "Keyset 2",  "Gamepad 1", "Gamepad 2"};
  ImGui::Combo("##Port1", &port1_device_, devices, IM_ARRAYSIZE(devices));
  ImGui::SameLine();
  ImGui::SetNextItemWidth(100);
  ImGui::Combo("##Port2", &port2_device_, devices, IM_ARRAYSIZE(devices));
  ImGui::SameLine();

  if (ImGui::Button("Keyboard")) {
    show_keyboard_ = !show_keyboard_;
  }
  ImGui::SameLine();

  ImGui::BeginGroup();
  if (ImGui::Button(emulator_.isRunning() ? "Pause" : "Run")) {
    ToggleRunPause();
  }
  ImGui::SameLine();
  if (ImGui::Button("Reset")) {
    HardReset();
  }
  ImGui::SameLine();
  if (ImGui::Button("Power")) {
    TogglePower();
  }
  ImGui::EndGroup();

  ImGui::End();

  if (show_settings_) {
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Settings", &show_settings_)) {
      if (ImGui::BeginTabBar("SettingsTabs")) {
        if (ImGui::BeginTabItem("ROMs")) {
          ImGui::Spacing();
          auto rt = emulator_.mem.getRomTraits();
          DrawRomInfo(
              "KICKSTART ROM", rt, rt.crc != 0, &kickstart_path_,
              [this]() { LoadKickstart(kickstart_path_); },
              [this]() { EjectKickstart(); });
          ImGui::Separator();
          auto et = emulator_.mem.getExtTraits();
          DrawRomInfo(
              "EXTENDED ROM", et, et.crc != 0, &ext_rom_path_,
              [this]() { LoadExtendedRom(ext_rom_path_); },
              [this]() { EjectExtendedRom(); });
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Audio")) {
          ImGui::Spacing();
          if (ImGui::SliderInt("Volume", &volume_, 0, 100, "%d%%")) {
            emulator_.set(vamiga::Opt::AUD_VOLL, volume_);
            emulator_.set(vamiga::Opt::AUD_VOLR, volume_);
            config_->SetInt(gui::ConfigKeys::kAudioVolume, volume_);
          }
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Input")) {
          ImGui::Spacing();
          if (ImGui::Checkbox("Pause in background",
                              &input_manager_->pause_in_background_)) {
            SaveConfig();
          }
          ImGui::Separator();
          ImGui::Text("Mouse Capture");
          if (ImGui::Checkbox("Retain mouse by clicking in window",
                              &input_manager_->retain_mouse_by_click_)) {
            SaveConfig();
          }
          if (ImGui::Checkbox("Retain mouse by entering window",
                              &input_manager_->retain_mouse_by_entering_)) {
            SaveConfig();
          }
          ImGui::Separator();
          ImGui::Text("Mouse Release");
          if (ImGui::Checkbox("Release mouse by shaking",
                              &input_manager_->release_mouse_by_shaking_)) {
            SaveConfig();
          }
          ImGui::TextDisabled(
              "Note: You can always release the mouse by pressing Ctrl+G");
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Media & Controls")) {
          for (int i = 0; i < 4; ++i) {
            ImGui::PushID(i);
            ImGui::Text("DF%d:", i);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-120);
            ImGui::InputText("##path", &floppy_paths_[i]);
            ImGui::SameLine();
            if (ImGui::Button("Insert", ImVec2(50, 0))) {
              gui::PickerOptions opts;
              opts.title = std::format("Select Floppy DF{}", i);
              opts.filters = "Disk Files (*.adf){.adf}";
              gui::FilePicker::Instance().Open(
                  std::format("Floppy{}", i), opts,
                  [this, i](auto p) { InsertFloppy(i, p); });
            }
            ImGui::SameLine();
            if (ImGui::Button("Eject", ImVec2(50, 0))) EjectFloppy(i);
            ImGui::PopID();
          }
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }
    }
    ImGui::End();
  }

  if (show_inspector_)
    gui::Inspector::Instance().Draw(&show_inspector_, emulator_);
  if (show_dashboard_)
    gui::Dashboard::Instance().Draw(&show_dashboard_, emulator_);
  if (show_console_) gui::Console::Instance().Draw(&show_console_, emulator_);
  if (show_keyboard_)
    gui::VirtualKeyboard::Instance().Draw(&show_keyboard_, emulator_);
  gui::FilePicker::Instance().Draw();
}

void Application::LoadKickstart(const std::filesystem::path& path) {
  try {
    emulator_.mem.loadRom(path);
    emulator_.hardReset();
    config_->SetString(gui::ConfigKeys::kKickstartPath, path.string());
    kickstart_path_ = path.string();
  } catch (...) {
  }
}
void Application::EjectKickstart() {
  emulator_.mem.deleteRom();
  emulator_.hardReset();
  config_->SetString(gui::ConfigKeys::kKickstartPath, "");
  kickstart_path_ = "";
}
void Application::LoadExtendedRom(const std::filesystem::path& path) {
  try {
    emulator_.mem.loadExt(path);
    emulator_.hardReset();
    config_->SetString(gui::ConfigKeys::kExtRomPath, path.string());
    ext_rom_path_ = path.string();
  } catch (...) {
  }
}
void Application::EjectExtendedRom() {
  emulator_.mem.deleteExt();
  emulator_.hardReset();
  config_->SetString(gui::ConfigKeys::kExtRomPath, "");
  ext_rom_path_ = "";
}
void Application::InsertFloppy(int drive, const std::filesystem::path& path) {
  if (drive < 0 || drive > 3) return;
  try {
    emulator_.df[drive]->insert(path, false);
    config_->SetFloppyPath(drive, path.string());
    floppy_paths_[drive] = path.string();
  } catch (...) {
  }
}
void Application::EjectFloppy(int drive) {
  if (drive < 0 || drive > 3) return;
  emulator_.df[drive]->ejectDisk();
  config_->SetFloppyPath(drive, "");
  floppy_paths_[drive] = "";
}
void Application::TogglePower() {
  if (emulator_.isPoweredOn())
    emulator_.powerOff();
  else
    emulator_.run();
}
void Application::HardReset() { emulator_.hardReset(); }
void Application::ToggleRunPause() {
  if (emulator_.isRunning())
    emulator_.pause();
  else
    emulator_.run();
}
void Application::LoadSnapshot(const std::filesystem::path& path) {
  try {
    emulator_.amiga.loadSnapshot(path);
  } catch (...) {
  }
}
void Application::SaveSnapshot(const std::filesystem::path& path) {
  try {
    emulator_.amiga.saveSnapshot(path);
  } catch (...) {
  }
}

namespace core_actions {
std::string_view GetRetroShellText() { return ""; }
void RetroShellPressKey(vamiga::RSKey key, bool shift) {}
void RetroShellPressChar(char c) {}
void RetroShellPressString(const std::string& s) {}
}  // namespace core_actions
