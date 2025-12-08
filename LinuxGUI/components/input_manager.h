#ifndef LINUXGUI_COMPONENTS_INPUT_MANAGER_H_
#define LINUXGUI_COMPONENTS_INPUT_MANAGER_H_

#include <SDL.h>
#include <imgui.h>

#include <map>
#include <memory>
#include <vector>

#include "VAmiga.h"

struct SDLGameControllerDeleter {
  void operator()(SDL_GameController* c) const {
    if (c) SDL_GameControllerClose(c);
  }
};

class InputManager {
 public:
  explicit InputManager(vamiga::VAmiga& emulator);
  ~InputManager();

  void HandleEvent(const SDL_Event& event);
  void Update();

  bool IsCaptured() const { return captured_; }
  void SetCaptured(bool captured);

  void SetViewportHovered(bool hovered) { viewport_hovered_ = hovered; }
  void HandleWindowFocus(bool focused);

  void SetPortDevices(int p1, int p2) {
    port1_device_ = p1;
    port2_device_ = p2;
  }

  bool retain_mouse_by_click_ = true;
  bool retain_mouse_by_entering_ = false;
  bool release_mouse_by_shaking_ = true;
  bool pause_in_background_ = true;

 private:
  void HandleKeyDown(const SDL_KeyboardEvent& event);
  void HandleKeyUp(const SDL_KeyboardEvent& event);
  void HandleMouseButtonDown(const SDL_MouseButtonEvent& event);
  void HandleMouseButtonUp(const SDL_MouseButtonEvent& event);
  void HandleMouseMotion(const SDL_MouseMotionEvent& event);

  void HandleControllerDeviceAdded(const SDL_ControllerDeviceEvent& event);
  void HandleControllerDeviceRemoved(const SDL_ControllerDeviceEvent& event);
  void HandleControllerButton(const SDL_ControllerButtonEvent& event);
  void HandleControllerAxis(const SDL_ControllerAxisEvent& event);

  bool IsReleaseKeyCombo(const SDL_KeyboardEvent& event);
  bool IsGrabKeyCombo(const SDL_KeyboardEvent& event);
  bool HandleKeyset(int device_id, const SDL_KeyboardEvent& event,
                    bool is_down);

  vamiga::MouseAPI* GetActiveMouse();
  vamiga::KeyCode SdlToAmigaKeyCode(SDL_Keycode key);

  vamiga::VAmiga& emulator_;
  bool captured_ = false;
  bool viewport_hovered_ = false;
  bool was_paused_by_focus_loss_ = false;

  int port1_device_ = 1;
  int port2_device_ = 2;

  std::map<SDL_JoystickID,
           std::unique_ptr<SDL_GameController, SDLGameControllerDeleter>>
      controllers_;
  std::vector<SDL_JoystickID> gamepad_ids_;
};

#endif  // LINUXGUI_COMPONENTS_INPUT_MANAGER_H_
