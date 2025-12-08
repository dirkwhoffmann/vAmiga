#include "virtual_keyboard.h"

#include <format>
#include <string>

#include "Components/AmigaTypes.h"

namespace gui {

VirtualKeyboard& VirtualKeyboard::Instance() {
  static VirtualKeyboard instance;
  return instance;
}

void VirtualKeyboard::DrawKey(vamiga::VAmiga& emu, const char* label, int width,
                              int code) {
  if (ImGui::Button(label, ImVec2(static_cast<float>(width), 0.0f))) {
    if (code >= 0) {
      emu.keyboard.press(static_cast<vamiga::KeyCode>(code));
      emu.keyboard.release(static_cast<vamiga::KeyCode>(code));
    }
  }
  if (ImGui::IsItemActive() && code >= 0) {
  }
}

void VirtualKeyboard::Draw(bool* p_open, vamiga::VAmiga& emu) {
  if (!p_open || !*p_open) return;

  ImGui::SetNextWindowSize(ImVec2(800, 300), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Virtual Keyboard", p_open, ImGuiWindowFlags_NoResize)) {
    ImGui::End();
    return;
  }

  const int kw = 40;
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));

  DrawKey(emu, "ESC", kw, 0x45);
  ImGui::SameLine();
  for (int i = 0; i < 10; i++) {
    std::string buf = std::format("F{{}}", i + 1);
    DrawKey(emu, buf.c_str(), kw, 0x50 + i);
    ImGui::SameLine();
  }
  DrawKey(emu, "DEL", kw, 0x46);
  ImGui::NewLine();

  DrawKey(emu, "`", kw, 0x00);
  ImGui::SameLine();
  for (int i = 1; i <= 9; i++) {
    char c = static_cast<char>('0' + i);
    std::string buf = std::format("{}", c);
    DrawKey(emu, buf.c_str(), kw, i);
    ImGui::SameLine();
  }
  DrawKey(emu, "0", kw, 0x0A);
  ImGui::SameLine();
  DrawKey(emu, "-", kw, 0x0B);
  ImGui::SameLine();
  DrawKey(emu, "=", kw, 0x0C);
  ImGui::SameLine();
  DrawKey(emu, "\\", kw, 0x0D);
  ImGui::SameLine();
  DrawKey(emu, "<-", kw * 2, 0x41);
  ImGui::NewLine();

  DrawKey(emu, "TAB", static_cast<int>(kw * 1.5), 0x42);
  ImGui::SameLine();
  DrawKey(emu, "Q", kw, 0x10);
  ImGui::SameLine();
  DrawKey(emu, "W", kw, 0x11);
  ImGui::SameLine();
  DrawKey(emu, "E", kw, 0x12);
  ImGui::SameLine();
  DrawKey(emu, "R", kw, 0x13);
  ImGui::SameLine();
  DrawKey(emu, "T", kw, 0x14);
  ImGui::SameLine();
  DrawKey(emu, "Y", kw, 0x15);
  ImGui::SameLine();
  DrawKey(emu, "U", kw, 0x16);
  ImGui::SameLine();
  DrawKey(emu, "I", kw, 0x17);
  ImGui::SameLine();
  DrawKey(emu, "O", kw, 0x18);
  ImGui::SameLine();
  DrawKey(emu, "P", kw, 0x19);
  ImGui::SameLine();
  DrawKey(emu, "[", kw, 0x1A);
  ImGui::SameLine();
  DrawKey(emu, "]", kw, 0x1B);
  ImGui::SameLine();
  DrawKey(emu, "RET", static_cast<int>(kw * 1.5), 0x44);
  ImGui::NewLine();

  DrawKey(emu, "CTRL", static_cast<int>(kw * 1.8), 0x63);
  ImGui::SameLine();
  DrawKey(emu, "A", kw, 0x20);
  ImGui::SameLine();
  DrawKey(emu, "S", kw, 0x21);
  ImGui::SameLine();
  DrawKey(emu, "D", kw, 0x22);
  ImGui::SameLine();
  DrawKey(emu, "F", kw, 0x23);
  ImGui::SameLine();
  DrawKey(emu, "G", kw, 0x24);
  ImGui::SameLine();
  DrawKey(emu, "H", kw, 0x25);
  ImGui::SameLine();
  DrawKey(emu, "J", kw, 0x26);
  ImGui::SameLine();
  DrawKey(emu, "K", kw, 0x27);
  ImGui::SameLine();
  DrawKey(emu, "L", kw, 0x28);
  ImGui::SameLine();
  DrawKey(emu, ";", kw, 0x29);
  ImGui::SameLine();
  DrawKey(emu, "'", kw, 0x2A);
  ImGui::SameLine();
  DrawKey(emu, "#", kw, 0x2B);
  ImGui::NewLine();

  DrawKey(emu, "SHIFT", static_cast<int>(kw * 2.3), 0x60);
  ImGui::SameLine();
  DrawKey(emu, "<", kw, 0x30);
  ImGui::SameLine();
  DrawKey(emu, "Z", kw, 0x31);
  ImGui::SameLine();
  DrawKey(emu, "X", kw, 0x32);
  ImGui::SameLine();
  DrawKey(emu, "C", kw, 0x33);
  ImGui::SameLine();
  DrawKey(emu, "V", kw, 0x34);
  ImGui::SameLine();
  DrawKey(emu, "B", kw, 0x35);
  ImGui::SameLine();
  DrawKey(emu, "N", kw, 0x36);
  ImGui::SameLine();
  DrawKey(emu, "M", kw, 0x37);
  ImGui::SameLine();
  DrawKey(emu, ",", kw, 0x38);
  ImGui::SameLine();
  DrawKey(emu, ".", kw, 0x39);
  ImGui::SameLine();
  DrawKey(emu, "/", kw, 0x3A);
  ImGui::SameLine();
  DrawKey(emu, "SHIFT", static_cast<int>(kw * 2.3), 0x61);
  ImGui::NewLine();

  DrawKey(emu, "ALT", static_cast<int>(kw * 1.5), 0x64);
  ImGui::SameLine();
  DrawKey(emu, "L-A", static_cast<int>(kw * 1.5), 0x66);
  ImGui::SameLine();
  DrawKey(emu, "SPACE", kw * 7, 0x40);
  ImGui::SameLine();
  DrawKey(emu, "R-A", static_cast<int>(kw * 1.5), 0x67);
  ImGui::SameLine();
  DrawKey(emu, "ALT", static_cast<int>(kw * 1.5), 0x65);

  ImGui::PopStyleVar();
  ImGui::End();
}

}  // namespace gui
