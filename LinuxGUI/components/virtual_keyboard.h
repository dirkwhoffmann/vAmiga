#ifndef LINUXGUI_COMPONENTS_VIRTUAL_KEYBOARD_H_
#define LINUXGUI_COMPONENTS_VIRTUAL_KEYBOARD_H_

#include "VAmiga.h"
#include "imgui.h"

namespace gui {

class VirtualKeyboard {
 public:
  static VirtualKeyboard& Instance();

  void Draw(bool* p_open, vamiga::VAmiga& emu);

 private:
  VirtualKeyboard() = default;

  void DrawKey(vamiga::VAmiga& emu, const char* label, int width, int code);
};

}  // namespace gui

#endif  // LINUXGUI_COMPONENTS_VIRTUAL_KEYBOARD_H_
