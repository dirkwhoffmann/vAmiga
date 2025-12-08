#include "dashboard.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <format>
#include <string>

namespace gui {

Dashboard& Dashboard::Instance() {
  static Dashboard instance;
  return instance;
}

Dashboard::Dashboard() {
  cpu_load_.resize(kHistorySize, 0.0f);
  gpu_fps_.resize(kHistorySize, 0.0f);
  emu_fps_.resize(kHistorySize, 0.0f);
  chip_ram_activity_.resize(kHistorySize, 0.0f);
  slow_ram_activity_.resize(kHistorySize, 0.0f);
  fast_ram_activity_.resize(kHistorySize, 0.0f);
  audio_buffer_fill_.resize(kHistorySize, 0.0f);

  audio_waveform_buffer_.resize(300 * 100);
}

void Dashboard::UpdateData(vamiga::VAmiga& emu) {
  auto shift = [](std::vector<float>& v, float new_val) {
    std::rotate(v.begin(), v.begin() + 1, v.end());
    v.back() = new_val;
  };

  auto stats = emu.getStats();
  shift(cpu_load_, (float)stats.cpuLoad * 100.0f);

  auto mem_stats = emu.mem.getStats();
  shift(chip_ram_activity_,
        (float)(mem_stats.chipReads.accumulated +
                mem_stats.chipWrites.accumulated));
  shift(slow_ram_activity_,
        (float)(mem_stats.slowReads.accumulated +
                mem_stats.slowWrites.accumulated));
  shift(fast_ram_activity_,
        (float)(mem_stats.fastReads.accumulated +
                mem_stats.fastWrites.accumulated));

  auto audio_stats = emu.audioPort.getStats();
  shift(audio_buffer_fill_, (float)(audio_stats.fillLevel * 100.0f));
}

void Dashboard::DrawPlot(const char* label, const std::vector<float>& data,
                         float min, float max, const char* overlay_text) {
  ImGui::PlotLines(label, data.data(), (int)data.size(), 0, overlay_text, min,
                   max, ImVec2(0, 80));
}

void Dashboard::Draw(bool* p_open, vamiga::VAmiga& emu) {
  if (!p_open || !*p_open) return;

  UpdateData(emu);

  ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Dashboard", p_open)) {
    if (ImGui::CollapsingHeader("Host System", ImGuiTreeNodeFlags_DefaultOpen)) {
      std::string buf = std::format("CPU Load: {:.1f}%", cpu_load_.back());
      DrawPlot("##cpu", cpu_load_, 0.0f, 100.0f, buf.c_str());
    }

    if (ImGui::CollapsingHeader("Memory Activity",
                                ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Text("Chip RAM");
      DrawPlot("##chip", chip_ram_activity_, 0.0f, FLT_MAX);

      ImGui::Text("Slow RAM");
      DrawPlot("##slow", slow_ram_activity_, 0.0f, FLT_MAX);

      ImGui::Text("Fast RAM");
      DrawPlot("##fast", fast_ram_activity_, 0.0f, FLT_MAX);
    }

    if (ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_DefaultOpen)) {
      std::string buf =
          std::format("Buffer: {:.1f}%", audio_buffer_fill_.back());
      DrawPlot("##audio_fill", audio_buffer_fill_, 0.0f, 100.0f, buf.c_str());

      ImGui::TextDisabled(
          "Waveform visualization requires texture update logic.");
    }
  }
  ImGui::End();
}

}  // namespace gui
