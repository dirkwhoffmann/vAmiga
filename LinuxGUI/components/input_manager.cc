#include "input_manager.h"

#include <algorithm>
#include <iostream>

#include "../Core/Peripherals/Joystick/JoystickTypes.h"

InputManager::InputManager(vamiga::VAmiga& emulator) : emulator_(emulator) {
  int num_joysticks = SDL_NumJoysticks();
  for (int i = 0; i < num_joysticks; ++i) {
    if (SDL_IsGameController(i)) {
      SDL_ControllerDeviceEvent ev;
      ev.type = SDL_CONTROLLERDEVICEADDED;
      ev.which = i;
      HandleControllerDeviceAdded(ev);
    }
  }
}

InputManager::~InputManager() { controllers_.clear(); }

void InputManager::SetCaptured(bool captured) {
  if (captured_ != captured) {
    captured_ = captured;
    SDL_SetRelativeMouseMode(captured_ ? SDL_TRUE : SDL_FALSE);
  }
}

void InputManager::HandleWindowFocus(bool focused) {
  if (focused) {
    if (pause_in_background_ && was_paused_by_focus_loss_) {
      if (!emulator_.isRunning()) {
        emulator_.run();
      }
      was_paused_by_focus_loss_ = false;
    }
  } else {
    if (captured_) {
      SetCaptured(false);
    }
    if (pause_in_background_) {
      if (emulator_.isRunning()) {
        emulator_.pause();
        was_paused_by_focus_loss_ = true;
      }
    }
  }
}

void InputManager::Update() {}

void InputManager::HandleEvent(const SDL_Event& event) {
  ImGuiIO& io = ImGui::GetIO();

  if (event.type == SDL_KEYDOWN) {
    if (IsGrabKeyCombo(event.key)) {
      SetCaptured(!captured_);
      return;
    }
  }

  if (!captured_) {
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEWHEEL) {
      if (io.WantCaptureMouse && !viewport_hovered_) return;
    }
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
      if (io.WantCaptureKeyboard) return;
    }
  }

  switch (event.type) {
    case SDL_KEYDOWN:
      HandleKeyDown(event.key);
      break;
    case SDL_KEYUP:
      HandleKeyUp(event.key);
      break;
    case SDL_MOUSEBUTTONDOWN:
      HandleMouseButtonDown(event.button);
      break;
    case SDL_MOUSEBUTTONUP:
      HandleMouseButtonUp(event.button);
      break;
    case SDL_MOUSEMOTION:
      HandleMouseMotion(event.motion);
      break;
    case SDL_CONTROLLERDEVICEADDED:
      HandleControllerDeviceAdded(event.cdevice);
      break;
    case SDL_CONTROLLERDEVICEREMOVED:
      HandleControllerDeviceRemoved(event.cdevice);
      break;
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
      HandleControllerButton(event.cbutton);
      break;
    case SDL_CONTROLLERAXISMOTION:
      HandleControllerAxis(event.caxis);
      break;
  }
}

vamiga::MouseAPI* InputManager::GetActiveMouse() {
  if (port1_device_ == 1) return &emulator_.controlPort1.mouse;
  if (port2_device_ == 1) return &emulator_.controlPort2.mouse;
  return &emulator_.controlPort1.mouse;
}

bool InputManager::HandleKeyset(int device_id, const SDL_KeyboardEvent& event,
                                bool is_down) {
  using namespace vamiga;

  JoystickAPI* joy = nullptr;
  if (port1_device_ == device_id)
    joy = &emulator_.controlPort1.joystick;
  else if (port2_device_ == device_id)
    joy = &emulator_.controlPort2.joystick;

  if (!joy) return false;

  SDL_Keycode sym = event.keysym.sym;
  bool hit = false;

  if (device_id == 2) {
    if (sym == SDLK_UP) {
      joy->trigger(is_down ? GamePadAction::PULL_UP
                           : GamePadAction::RELEASE_Y);
      hit = true;
    } else if (sym == SDLK_DOWN) {
      joy->trigger(is_down ? GamePadAction::PULL_DOWN
                           : GamePadAction::RELEASE_Y);
      hit = true;
    } else if (sym == SDLK_LEFT) {
      joy->trigger(is_down ? GamePadAction::PULL_LEFT
                           : GamePadAction::RELEASE_X);
      hit = true;
    } else if (sym == SDLK_RIGHT) {
      joy->trigger(is_down ? GamePadAction::PULL_RIGHT
                           : GamePadAction::RELEASE_X);
      hit = true;
    } else if (sym == SDLK_RCTRL || sym == SDLK_KP_0) {
      joy->trigger(is_down ? GamePadAction::PRESS_FIRE
                           : GamePadAction::RELEASE_FIRE);
      hit = true;
    }
  } else if (device_id == 3) {
    if (sym == SDLK_w) {
      joy->trigger(is_down ? GamePadAction::PULL_UP
                           : GamePadAction::RELEASE_Y);
      hit = true;
    } else if (sym == SDLK_s) {
      joy->trigger(is_down ? GamePadAction::PULL_DOWN
                           : GamePadAction::RELEASE_Y);
      hit = true;
    } else if (sym == SDLK_a) {
      joy->trigger(is_down ? GamePadAction::PULL_LEFT
                           : GamePadAction::RELEASE_X);
      hit = true;
    } else if (sym == SDLK_d) {
      joy->trigger(is_down ? GamePadAction::PULL_RIGHT
                           : GamePadAction::RELEASE_X);
      hit = true;
    } else if (sym == SDLK_LCTRL) {
      joy->trigger(is_down ? GamePadAction::PRESS_FIRE
                           : GamePadAction::RELEASE_FIRE);
      hit = true;
    }
  }
  return hit;
}

void InputManager::HandleKeyDown(const SDL_KeyboardEvent& event) {
  if (captured_) {
    if (IsReleaseKeyCombo(event)) {
      SetCaptured(false);
      return;
    }
  }

  if (HandleKeyset(2, event, true)) return;
  if (HandleKeyset(3, event, true)) return;

  vamiga::KeyCode kc = SdlToAmigaKeyCode(event.keysym.sym);
  if (kc != 0xFF) {
    emulator_.keyboard.press(kc);
  }
}

void InputManager::HandleKeyUp(const SDL_KeyboardEvent& event) {
  if (HandleKeyset(2, event, false)) return;
  if (HandleKeyset(3, event, false)) return;

  vamiga::KeyCode kc = SdlToAmigaKeyCode(event.keysym.sym);
  if (kc != 0xFF) {
    emulator_.keyboard.release(kc);
  }
}

void InputManager::HandleMouseButtonDown(const SDL_MouseButtonEvent& event) {
  using namespace vamiga;
  if (!captured_) {
    if (retain_mouse_by_click_ && event.button == SDL_BUTTON_LEFT &&
        viewport_hovered_) {
      SetCaptured(true);
    }
  } else {
    MouseAPI* mouse = GetActiveMouse();
    if (mouse) {
      if (event.button == SDL_BUTTON_LEFT)
        mouse->trigger(GamePadAction::PRESS_LEFT);
      else if (event.button == SDL_BUTTON_RIGHT)
        mouse->trigger(GamePadAction::PRESS_RIGHT);
      else if (event.button == SDL_BUTTON_MIDDLE)
        mouse->trigger(GamePadAction::PRESS_MIDDLE);
    }
  }
}

void InputManager::HandleMouseButtonUp(const SDL_MouseButtonEvent& event) {
  using namespace vamiga;
  if (captured_) {
    MouseAPI* mouse = GetActiveMouse();
    if (mouse) {
      if (event.button == SDL_BUTTON_LEFT)
        mouse->trigger(GamePadAction::RELEASE_LEFT);
      else if (event.button == SDL_BUTTON_RIGHT)
        mouse->trigger(GamePadAction::RELEASE_RIGHT);
      else if (event.button == SDL_BUTTON_MIDDLE)
        mouse->trigger(GamePadAction::RELEASE_MIDDLE);
    }
  }
}

void InputManager::HandleMouseMotion(const SDL_MouseMotionEvent& event) {
  if (!captured_) {
    if (retain_mouse_by_entering_ && viewport_hovered_) {
      SetCaptured(true);
      return;
    }
  }

  if (captured_) {
    float scale_x = 1.0f;
    float scale_y = 1.0f;
    double dx = event.xrel * scale_x;
    double dy = event.yrel * scale_y;

    vamiga::MouseAPI* mouse = GetActiveMouse();
    if (mouse) {
      mouse->setDxDy(dx, dy);

      if (release_mouse_by_shaking_) {
        if (mouse->detectShakeDxDy(dx, dy)) {
          SetCaptured(false);
        }
      }
    }
  }
}

void InputManager::HandleControllerDeviceAdded(
    const SDL_ControllerDeviceEvent& event) {
  int device_index = event.which;
  if (SDL_IsGameController(device_index)) {
    SDL_GameController* controller = SDL_GameControllerOpen(device_index);
    if (controller) {
      SDL_Joystick* joy = SDL_GameControllerGetJoystick(controller);
      SDL_JoystickID instance_id = SDL_JoystickInstanceID(joy);

      controllers_[instance_id].reset(controller);

      gamepad_ids_.push_back(instance_id);
      std::cout << "Gamepad added: ID " << instance_id << " (Slot "
                << (gamepad_ids_.size() - 1) << ")" << std::endl;
    }
  }
}

void InputManager::HandleControllerDeviceRemoved(
    const SDL_ControllerDeviceEvent& event) {
  SDL_JoystickID instance_id = event.which;
  auto it = controllers_.find(instance_id);
  if (it != controllers_.end()) {
    controllers_.erase(it);

    auto vec_it =
        std::find(gamepad_ids_.begin(), gamepad_ids_.end(), instance_id);
    if (vec_it != gamepad_ids_.end()) {
      gamepad_ids_.erase(vec_it);
    }
    std::cout << "Gamepad removed: ID " << instance_id << std::endl;
  }
}

void InputManager::HandleControllerButton(
    const SDL_ControllerButtonEvent& event) {
  int slot = -1;
  for (size_t i = 0; i < gamepad_ids_.size(); ++i) {
    if (gamepad_ids_[i] == event.which) {
      slot = (int)i;
      break;
    }
  }
  if (slot == -1) return;

  vamiga::JoystickAPI* joy = nullptr;
  if (port1_device_ == (4 + slot))
    joy = &emulator_.controlPort1.joystick;
  else if (port2_device_ == (4 + slot))
    joy = &emulator_.controlPort2.joystick;

  if (!joy) return;

  using namespace vamiga;
  bool down = (event.state == SDL_PRESSED);

  switch (event.button) {
    case SDL_CONTROLLER_BUTTON_A:
    case SDL_CONTROLLER_BUTTON_B:
    case SDL_CONTROLLER_BUTTON_X:
    case SDL_CONTROLLER_BUTTON_Y:
      joy->trigger(down ? GamePadAction::PRESS_FIRE
                        : GamePadAction::RELEASE_FIRE);
      break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
      joy->trigger(down ? GamePadAction::PULL_UP : GamePadAction::RELEASE_Y);
      break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
      joy->trigger(down ? GamePadAction::PULL_DOWN : GamePadAction::RELEASE_Y);
      break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
      joy->trigger(down ? GamePadAction::PULL_LEFT : GamePadAction::RELEASE_X);
      break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
      joy->trigger(down ? GamePadAction::PULL_RIGHT : GamePadAction::RELEASE_X);
      break;
  }
}

void InputManager::HandleControllerAxis(const SDL_ControllerAxisEvent& event) {
  int slot = -1;
  for (size_t i = 0; i < gamepad_ids_.size(); ++i) {
    if (gamepad_ids_[i] == event.which) {
      slot = (int)i;
      break;
    }
  }
  if (slot == -1) return;

  vamiga::JoystickAPI* joy = nullptr;
  if (port1_device_ == (4 + slot))
    joy = &emulator_.controlPort1.joystick;
  else if (port2_device_ == (4 + slot))
    joy = &emulator_.controlPort2.joystick;

  if (!joy) return;

  using namespace vamiga;

  const int kThreshold = 16000;

  if (event.axis == SDL_CONTROLLER_AXIS_LEFTY) {
    if (event.value < -kThreshold)
      joy->trigger(GamePadAction::PULL_UP);
    else if (event.value > kThreshold)
      joy->trigger(GamePadAction::PULL_DOWN);
    else
      joy->trigger(GamePadAction::RELEASE_Y);
  } else if (event.axis == SDL_CONTROLLER_AXIS_LEFTX) {
    if (event.value < -kThreshold)
      joy->trigger(GamePadAction::PULL_LEFT);
    else if (event.value > kThreshold)
      joy->trigger(GamePadAction::PULL_RIGHT);
    else
      joy->trigger(GamePadAction::RELEASE_X);
  }
}

bool InputManager::IsGrabKeyCombo(const SDL_KeyboardEvent& event) {
  bool ctrl =
      (event.keysym.mod & KMOD_LCTRL) || (event.keysym.mod & KMOD_RCTRL);
  return ctrl && event.keysym.sym == SDLK_g;
}

bool InputManager::IsReleaseKeyCombo(const SDL_KeyboardEvent& event) {
  if (IsGrabKeyCombo(event)) return true;
  bool ctrl = (event.keysym.mod & KMOD_CTRL);
  bool alt = (event.keysym.mod & KMOD_ALT);
  if (ctrl && alt) return true;
  return false;
}

vamiga::KeyCode InputManager::SdlToAmigaKeyCode(SDL_Keycode key) {
  using namespace vamiga;
  switch (key) {
    case SDLK_BACKQUOTE: return 0x00;
    case SDLK_1: return 0x01;
    case SDLK_2: return 0x02;
    case SDLK_3: return 0x03;
    case SDLK_4: return 0x04;
    case SDLK_5: return 0x05;
    case SDLK_6: return 0x06;
    case SDLK_7: return 0x07;
    case SDLK_8: return 0x08;
    case SDLK_9: return 0x09;
    case SDLK_0: return 0x0A;
    case SDLK_MINUS: return 0x0B;
    case SDLK_EQUALS: return 0x0C;
    case SDLK_BACKSLASH: return 0x0D;
    case SDLK_q: return 0x10;
    case SDLK_w: return 0x11;
    case SDLK_e: return 0x12;
    case SDLK_r: return 0x13;
    case SDLK_t: return 0x14;
    case SDLK_y: return 0x15;
    case SDLK_u: return 0x16;
    case SDLK_i: return 0x17;
    case SDLK_o: return 0x18;
    case SDLK_p: return 0x19;
    case SDLK_LEFTBRACKET: return 0x1A;
    case SDLK_RIGHTBRACKET: return 0x1B;
    case SDLK_a: return 0x20;
    case SDLK_s: return 0x21;
    case SDLK_d: return 0x22;
    case SDLK_f: return 0x23;
    case SDLK_g: return 0x24;
    case SDLK_h: return 0x25;
    case SDLK_j: return 0x26;
    case SDLK_k: return 0x27;
    case SDLK_l: return 0x28;
    case SDLK_SEMICOLON: return 0x29;
    case SDLK_QUOTE: return 0x2A;
    case SDLK_z: return 0x31;
    case SDLK_x: return 0x32;
    case SDLK_c: return 0x33;
    case SDLK_v: return 0x34;
    case SDLK_b: return 0x35;
    case SDLK_n: return 0x36;
    case SDLK_m: return 0x37;
    case SDLK_COMMA: return 0x38;
    case SDLK_PERIOD: return 0x39;
    case SDLK_SLASH: return 0x3A;
    case SDLK_SPACE: return 0x40;
    case SDLK_BACKSPACE: return 0x41;
    case SDLK_TAB: return 0x42;
    case SDLK_RETURN: return 0x44;
    case SDLK_ESCAPE: return 0x45;
    case SDLK_DELETE: return 0x46;
    case SDLK_UP: return 0x4C;
    case SDLK_DOWN: return 0x4D;
    case SDLK_RIGHT: return 0x4E;
    case SDLK_LEFT: return 0x4F;
    case SDLK_F1: return 0x50;
    case SDLK_F2: return 0x51;
    case SDLK_F3: return 0x52;
    case SDLK_F4: return 0x53;
    case SDLK_F5: return 0x54;
    case SDLK_F6: return 0x55;
    case SDLK_F7: return 0x56;
    case SDLK_F8: return 0x57;
    case SDLK_F9: return 0x58;
    case SDLK_F10: return 0x59;
    case SDLK_LSHIFT: return 0x60;
    case SDLK_RSHIFT: return 0x61;
    case SDLK_CAPSLOCK: return 0x62;
    case SDLK_LCTRL: return 0x63;
    case SDLK_LALT: return 0x64;
    case SDLK_RALT: return 0x65;
    case SDLK_LGUI: return 0x66;
    case SDLK_RGUI: return 0x67;
    default: return 0xFF;
  }
}