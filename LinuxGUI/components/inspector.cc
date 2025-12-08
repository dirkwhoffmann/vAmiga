#include "inspector.h"

#include <charconv>
#include <cstdio>

#include "Components/Agnus/AgnusTypes.h"
#include "Components/CIA/CIATypes.h"
#include "Components/CPU/CPUTypes.h"
#include "Components/Denise/DeniseTypes.h"
#include "Components/Memory/MemoryTypes.h"
#include "Components/Paula/PaulaTypes.h"

namespace gui {

Inspector& Inspector::Instance() {
  static Inspector instance;
  return instance;
}

Inspector::Inspector() {}

void Inspector::Register8(const char* label, uint8_t val) {
  ImGui::Text("%s: %02X", label, val);
}

void Inspector::Register16(const char* label, uint16_t val) {
  ImGui::Text("%s: %04X", label, val);
}

void Inspector::Register32(const char* label, uint32_t val) {
  ImGui::Text("%s: %08X", label, val);
}

void Inspector::Flag(const char* label, bool set) {
  if (set) {
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", label);
  } else {
    ImGui::TextDisabled("%s", label);
  }
}

void Inspector::DrawHexDump(vamiga::VAmiga& emu, uint32_t addr, int rows) {
  ImGui::BeginChild("HexDump", ImVec2(0, rows * ImGui::GetTextLineHeightWithSpacing()));
  for (int r = 0; r < rows; r++) {
    uint32_t row_addr = addr + (r * 16);
    ImGui::Text("%08X: ", row_addr);
    for (int c = 0; c < 16; c++) {
      ImGui::SameLine();
      uint8_t val = emu.mem.debugger.spypeek8(vamiga::Accessor::CPU, row_addr + c);
      ImGui::Text("%02X", val);
    }
    ImGui::SameLine();
    ImGui::Text(" ");
    for (int c = 0; c < 16; c++) {
      ImGui::SameLine();
      uint8_t val = emu.mem.debugger.spypeek8(vamiga::Accessor::CPU, row_addr + c);
      if (val >= 32 && val < 127)
        ImGui::Text("%c", (char)val);
      else
        ImGui::Text(".");
    }
  }
  ImGui::EndChild();
}

void Inspector::DrawCPU(vamiga::VAmiga& emu) {
  auto cpu_info = emu.cpu.getInfo();

  if (ImGui::CollapsingHeader("Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Columns(2, "CPU_Regs");
    Register32("D0", cpu_info.d[0]);
    Register32("A0", cpu_info.a[0]);
    Register32("D1", cpu_info.d[1]);
    Register32("A1", cpu_info.a[1]);
    Register32("D2", cpu_info.d[2]);
    Register32("A2", cpu_info.a[2]);
    Register32("D3", cpu_info.d[3]);
    Register32("A3", cpu_info.a[3]);
    Register32("D4", cpu_info.d[4]);
    Register32("A4", cpu_info.a[4]);
    Register32("D5", cpu_info.d[5]);
    Register32("A5", cpu_info.a[5]);
    Register32("D6", cpu_info.d[6]);
    Register32("A6", cpu_info.a[6]);
    Register32("D7", cpu_info.d[7]);
    Register32("A7", cpu_info.a[7]);
    ImGui::NextColumn();
    Register32("PC", cpu_info.pc0);
    Register16("SR", cpu_info.sr);
    Register32("USP", cpu_info.usp);
    Register32("ISP", cpu_info.isp);
    Register32("MSP", cpu_info.msp);
    Register32("VBR", cpu_info.vbr);
    Register32("CACR", cpu_info.cacr);
    Register32("CAAR", cpu_info.caar);
    ImGui::Columns(1);

    ImGui::Separator();
    ImGui::Text("Flags:");
    ImGui::SameLine();
    Flag("X", cpu_info.sr & 0x10);
    ImGui::SameLine();
    Flag("N", cpu_info.sr & 0x08);
    ImGui::SameLine();
    Flag("Z", cpu_info.sr & 0x04);
    ImGui::SameLine();
    Flag("V", cpu_info.sr & 0x02);
    ImGui::SameLine();
    Flag("C", cpu_info.sr & 0x01);
  }

  if (ImGui::CollapsingHeader("Control", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("Step Into")) {
      emu.stepInto();
    }
    ImGui::SameLine();
    if (ImGui::Button("Step Over")) {
      emu.stepOver();
    }
    ImGui::SameLine();
    if (ImGui::Button("Finish Line")) {
      emu.finishLine();
    }
    ImGui::SameLine();
    if (ImGui::Button("Finish Frame")) {
      emu.finishFrame();
    }
  }

  if (ImGui::CollapsingHeader("Disassembler", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Checkbox("Follow PC", &follow_pc_);
    if (follow_pc_) dasm_addr_ = cpu_info.pc0;

    vamiga::isize len;
    uint32_t addr = dasm_addr_;
    ImGui::BeginChild("Dasm", ImVec2(0, 200), true);
    for (int i = 0; i < 10; i++) {
      bool is_pc = (addr == cpu_info.pc0);
      if (is_pc) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));

      std::string instr = emu.cpu.debugger.disassembleInstr(addr, &len);
      ImGui::Text("%08X: %s", addr, instr.c_str());

      if (is_pc) ImGui::PopStyleColor();
      addr += len;
    }
    ImGui::EndChild();
  }

  if (ImGui::CollapsingHeader("Breakpoints")) {
    int count = emu.cpu.breakpoints.elements();
    for (int i = 0; i < count; i++) {
      auto info = emu.cpu.breakpoints.guardNr(i);
      if (info.has_value()) {
        ImGui::Text("BP %d: %08X (%s)", i, info->addr,
                    info->enabled ? "On" : "Off");
        ImGui::SameLine();
        if (ImGui::Button(
                (std::string("Toggle##") + std::to_string(i)).c_str())) {
          emu.cpu.breakpoints.toggle(i);
        }
      }
    }
    static char buf[16] = "";
    ImGui::InputText("Addr", buf, 16, ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::SameLine();
    if (ImGui::Button("Add BP")) {
      uint32_t addr = 0;
      std::from_chars(buf, buf + 16, addr, 16);
      emu.cpu.breakpoints.setAt(addr);
    }
  }
}

void Inspector::DrawMemory(vamiga::VAmiga& emu) {
  static char buf[16] = "";
  ImGui::InputText("Address", buf, 16, ImGuiInputTextFlags_CharsHexadecimal);
  if (ImGui::Button("Go")) {
    uint32_t addr = 0;
    std::from_chars(buf, buf + 16, addr, 16);
    mem_addr_ = addr;
  }

  DrawHexDump(emu, mem_addr_, 16);

  if (ImGui::CollapsingHeader("Memory Map")) {
    auto info = emu.mem.getInfo();
    if (info.chipMask > 0)
      ImGui::Text("Chip RAM:  %08X - %08X", 0, info.chipMask);
    if (info.hasRom)
      ImGui::Text("Kickstart: %08X - %08X", 0xF80000, 0xFFFFFF);
  }
}

void Inspector::DrawAgnus(vamiga::VAmiga& emu) {
  auto info = emu.agnus.getInfo();
  ImGui::Text("Position: V=%ld, H=%ld", info.vpos, info.hpos);

  if (ImGui::CollapsingHeader("Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
    Register16("DMACON", info.dmacon);
    Register16("INTENA", emu.paula.getInfo().intena);
    Register16("INTREQ", emu.paula.getInfo().intreq);
    Register16("ADKCON", emu.paula.getInfo().adkcon);
  }

  if (ImGui::CollapsingHeader("DMA Channels")) {
    Flag("DMAEN", info.dmacon & 0x0200);
    Flag("BPLEN", info.dmacon & 0x0100);
    Flag("COPEN", info.dmacon & 0x0080);
    Flag("BLTEN", info.dmacon & 0x0040);
    Flag("SPREN", info.dmacon & 0x0020);
    Flag("DSKEN", info.dmacon & 0x0010);
    Flag("AUD3", info.dmacon & 0x0008);
    Flag("AUD2", info.dmacon & 0x0004);
    Flag("AUD1", info.dmacon & 0x0002);
    Flag("AUD0", info.dmacon & 0x0001);
  }
}

void Inspector::DrawDenise(vamiga::VAmiga& emu) {
  auto info = emu.denise.getInfo();
  Register16("BPLCON0", info.bplcon0);
  Register16("BPLCON1", info.bplcon1);
  Register16("BPLCON2", info.bplcon2);
  Register16("DIWSTRT", info.diwstrt);
  Register16("DIWSTOP", info.diwstop);
  Register16("DDFSTRT", emu.agnus.getInfo().ddfstrt);
  Register16("DDFSTOP", emu.agnus.getInfo().ddfstop);
}

void Inspector::DrawPaula(vamiga::VAmiga& emu) {
  auto info = emu.paula.getInfo();
  if (ImGui::CollapsingHeader("Interrupts")) {
    Register16("INTENA", info.intena);
    Register16("INTREQ", info.intreq);
  }
}

void Inspector::DrawCIA(vamiga::VAmiga& emu) {
  auto cia_a = emu.ciaA.getInfo();
  auto cia_b = emu.ciaB.getInfo();

  if (ImGui::CollapsingHeader("CIA A (Odd)", ImGuiTreeNodeFlags_DefaultOpen)) {
    Register8("PRA", cia_a.portA.reg);
    Register8("PRB", cia_a.portB.reg);
    Register8("DDRA", cia_a.portA.dir);
    Register8("DDRB", cia_a.portB.dir);
    Register16("TA", cia_a.timerA.count);
    Register16("TB", cia_a.timerB.count);
    Register8("ICR", cia_a.icr);
  }
  if (ImGui::CollapsingHeader("CIA B (Even)", ImGuiTreeNodeFlags_DefaultOpen)) {
    Register8("PRA", cia_b.portA.reg);
    Register8("PRB", cia_b.portB.reg);
    Register16("TA", cia_b.timerA.count);
    Register16("TB", cia_b.timerB.count);
    Register8("ICR", cia_b.icr);
  }
}

void Inspector::DrawCopper(vamiga::VAmiga& emu) {
  auto info = emu.agnus.getInfo();
  Register32("COP1LC", info.audlc[0]);
  Register32("COPPC", info.coppc0);

  if (ImGui::CollapsingHeader("Copper List")) {
    uint32_t addr = info.coppc0;
    for (int i = 0; i < 8; i++) {
      std::string dis = emu.agnus.copper.disassemble(addr, true);
      ImGui::Text("%08X: %s", addr, dis.c_str());
      addr += 4;
    }
  }
}

void Inspector::DrawBlitter(vamiga::VAmiga& emu) {
  auto info = emu.agnus.getInfo();
  Register16("BLTCON0", info.bltcon0);
  Register16("BLTAMOD", info.bltamod);
  Register16("BLTBMOD", info.bltbmod);
  Register16("BLTCMOD", info.bltcmod);
  Register16("BLTDMOD", info.bltdmod);
  Register32("BLTAPT", info.bltpt[0]);
  Register32("BLTBPT", info.bltpt[1]);
  Register32("BLTCPT", info.bltpt[2]);
  Register32("BLTDPT", info.bltpt[3]);
}

void Inspector::DrawEvents(vamiga::VAmiga& emu) {
  auto info = emu.agnus.getInfo();
  if (ImGui::BeginTable("Events", 3)) {
    ImGui::TableSetupColumn("Slot");
    ImGui::TableSetupColumn("Event");
    ImGui::TableSetupColumn("Trigger");
    ImGui::TableHeadersRow();

    for (int i = 0; i < vamiga::SLOT_COUNT; i++) {
      auto& slot = info.slotInfo[i];
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", vamiga::EventSlotEnum::_key(slot.slot));
      ImGui::TableNextColumn();
      ImGui::Text("%s", slot.eventName ? slot.eventName : "-");
      ImGui::TableNextColumn();
      ImGui::Text("%lld", (long long)slot.trigger);
    }
    ImGui::EndTable();
  }
}

void Inspector::Draw(bool* p_open, vamiga::VAmiga& emu) {
  if (!p_open || !*p_open) return;

  ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Inspector", p_open)) {
    if (!emu.isTracking()) emu.trackOn();

    if (ImGui::BeginTabBar("InspectorTabs")) {
      if (ImGui::BeginTabItem("CPU")) {
        DrawCPU(emu);
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Memory")) {
        DrawMemory(emu);
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Agnus")) {
        DrawAgnus(emu);
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Denise")) {
        DrawDenise(emu);
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Paula")) {
        DrawPaula(emu);
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("CIA")) {
        DrawCIA(emu);
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Copper")) {
        DrawCopper(emu);
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Blitter")) {
        DrawBlitter(emu);
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Events")) {
        DrawEvents(emu);
        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }
  } else {
    if (emu.isTracking()) emu.trackOff();
  }
  ImGui::End();
}

}  // namespace gui
