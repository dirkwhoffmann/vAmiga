#ifndef LINUXGUI_COMPONENTS_DASHBOARD_H_
#define LINUXGUI_COMPONENTS_DASHBOARD_H_

#include <cstdint>
#include <vector>

#include "VAmiga.h"
#include "imgui.h"

namespace gui {

class Dashboard {
 public:
  static Dashboard& Instance();

  void Draw(bool* p_open, vamiga::VAmiga& emu);

 private:
  Dashboard();

  void DrawPlot(const char* label, const std::vector<float>& data, float min,
                float max, const char* overlay_text = nullptr);

  void UpdateData(vamiga::VAmiga& emu);

  static constexpr int kHistorySize = 100;
  std::vector<float> cpu_load_;
  std::vector<float> gpu_fps_;
  std::vector<float> emu_fps_;
  std::vector<float> chip_ram_activity_;
  std::vector<float> slow_ram_activity_;
  std::vector<float> fast_ram_activity_;
  std::vector<float> audio_buffer_fill_;

  std::vector<uint32_t> audio_waveform_buffer_;
};

}  // namespace gui

#endif  // LINUXGUI_COMPONENTS_DASHBOARD_H_
