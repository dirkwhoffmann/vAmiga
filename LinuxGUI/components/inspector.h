#ifndef LINUXGUI_COMPONENTS_INSPECTOR_H_
#define LINUXGUI_COMPONENTS_INSPECTOR_H_

#include <string>
#include <vector>

#include "VAmiga.h"
#include "imgui.h"

namespace gui {

class Inspector {
 public:
  static Inspector& Instance();

  void Draw(bool* p_open, vamiga::VAmiga& emu);

 private:
  Inspector();

  // Tabs
  void DrawCPU(vamiga::VAmiga& emu);
  void DrawMemory(vamiga::VAmiga& emu);
  void DrawAgnus(vamiga::VAmiga& emu);
  void DrawDenise(vamiga::VAmiga& emu);
  void DrawPaula(vamiga::VAmiga& emu);
  void DrawCIA(vamiga::VAmiga& emu);
  void DrawCopper(vamiga::VAmiga& emu);
  void DrawBlitter(vamiga::VAmiga& emu);
  void DrawEvents(vamiga::VAmiga& emu);

  // Helpers
  void Register8(const char* label, uint8_t val);
  void Register16(const char* label, uint16_t val);
  void Register32(const char* label, uint32_t val);
  void Flag(const char* label, bool set);

  // State for Disassembler
  int dasm_addr_ = 0;
  bool follow_pc_ = true;

  // State for Memory Editor
  int mem_addr_ = 0;
  int mem_rows_ = 16;

  // Helper for Hex Dump
  void DrawHexDump(vamiga::VAmiga& emu, uint32_t addr, int rows);
};

}  // namespace gui

#endif  // LINUXGUI_COMPONENTS_INSPECTOR_H_
