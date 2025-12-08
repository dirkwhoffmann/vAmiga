#ifndef LINUXGUI_COMPONENTS_CONSOLE_H_
#define LINUXGUI_COMPONENTS_CONSOLE_H_

#include <format>
#include <functional>
#include <string>
#include <vector>

#include "VAmiga.h"

namespace gui {

class Console {
 public:
  static Console& Instance();

  void Draw(bool* p_open, vamiga::VAmiga& emu);

  template <typename... Args>
  void AddLog(std::format_string<Args...> fmt, Args&&... args) {
    items_.push_back(std::format(fmt, std::forward<Args>(args)...));
    scroll_to_bottom_ = true;
  }

  void SetCommandCallback(std::function<void(const std::string&)> cb);

  void ExecCommand(std::string_view command_line);
  int TextEditCallback(void* data);

 private:
  Console();

  std::vector<char> input_buf_;
  std::vector<std::string> items_;
  bool scroll_to_bottom_;
  std::vector<std::string> history_;
  int history_pos_;
  std::vector<std::string> commands_;

  std::function<void(const std::string&)> command_callback_;
  std::string retro_shell_current_text_;
};

}  // namespace gui

#endif  // LINUXGUI_COMPONENTS_CONSOLE_H_
