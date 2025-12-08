#include "console.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string_view>

#include "core_actions.h"
#include "imgui.h"

namespace gui {

Console& Console::Instance() {
  static Console instance;
  return instance;
}

Console::Console()
    : input_buf_(256, 0), scroll_to_bottom_(true), history_pos_(-1) {
  AddLog("Welcome to vAmiga RetroShell!");

  commands_ = {"HELP", "HISTORY", "CLEAR", "CLASSIFY", "ECHO", "VAR", "TYPE",
               "MD",   "CLS",     "MEM",   "REGS",     "DISASM", "BP"};
}

void Console::SetCommandCallback(std::function<void(const std::string&)> cb) {
  command_callback_ = cb;
}

void Console::ExecCommand(std::string_view command_line) {
  AddLog("# {}\n", command_line);

  history_pos_ = -1;
  for (auto it = history_.begin(); it != history_.end(); ++it) {
    if (*it == command_line) {
      history_.erase(it);
      break;
    }
  }
  history_.emplace_back(command_line);

  if (command_callback_) {
    command_callback_(std::string(command_line));
  } else {
    AddLog("Unknown command: '{}'\n", command_line);
  }

  scroll_to_bottom_ = true;
}

static int TextEditCallbackStub(ImGuiInputTextCallbackData* data) {
  Console* console = (Console*)data->UserData;
  return console->TextEditCallback((void*)data);
}

int Console::TextEditCallback(void* data_void) {
  ImGuiInputTextCallbackData* data = (ImGuiInputTextCallbackData*)data_void;

  switch (data->EventFlag) {
    case ImGuiInputTextFlags_CallbackCompletion: {
      std::string_view buf(data->Buf, (size_t)data->BufTextLen);
      size_t cursor_pos = static_cast<size_t>(data->CursorPos);

      size_t word_start_pos =
          buf.find_last_of(" \t,;", cursor_pos > 0 ? cursor_pos - 1 : 0);
      if (word_start_pos == std::string_view::npos) {
        word_start_pos = 0;
      } else if (word_start_pos < cursor_pos) {
        word_start_pos += 1;
      }

      std::string_view word =
          buf.substr(word_start_pos, cursor_pos - word_start_pos);

      std::vector<std::string> candidates;
      for (const auto& command : commands_) {
        if (word.size() <= command.size() &&
            std::equal(word.begin(), word.end(), command.begin(),
                       [](char a, char b) {
                         return std::tolower((unsigned char)a) ==
                                std::tolower((unsigned char)b);
                       })) {
          candidates.push_back(command);
        }
      }

      if (candidates.empty()) {
        AddLog("No match for \"%s\"!\n", word.data());
      } else if (candidates.size() == 1) {
        data->DeleteChars((int)word_start_pos,
                          (int)(cursor_pos - word_start_pos));
        data->InsertChars(data->CursorPos, candidates[0].c_str());
        data->InsertChars(data->CursorPos, " ");
      } else {
        int match_len = (int)word.size();
        for (;;) {
          int c = 0;
          bool all_candidates_matches = true;
          for (size_t i = 0; i < candidates.size() && all_candidates_matches;
               i++)
            if (i == 0)
              c = std::toupper((unsigned char)candidates[i][match_len]);
            else if (c == 0 ||
                     c != std::toupper((unsigned char)candidates[i][match_len]))
              all_candidates_matches = false;
          if (!all_candidates_matches) break;
          match_len++;
        }

        if (match_len > 0) {
          data->DeleteChars((int)word_start_pos,
                            (int)(cursor_pos - word_start_pos));
          data->InsertChars(data->CursorPos, candidates[0].c_str(),
                            candidates[0].c_str() + match_len);
        }

        AddLog("Possible matches:\n");
        for (const auto& candidate : candidates)
          AddLog("- %s\n", candidate.c_str());
      }

      break;
    }
    case ImGuiInputTextFlags_CallbackHistory: {
      const int prev_history_pos = history_pos_;
      if (data->EventKey == ImGuiKey_UpArrow) {
        if (history_pos_ == -1)
          history_pos_ = (int)history_.size() - 1;
        else if (history_pos_ > 0)
          history_pos_--;
      } else if (data->EventKey == ImGuiKey_DownArrow) {
        if (history_pos_ != -1)
          if (++history_pos_ >= (int)history_.size()) history_pos_ = -1;
      }

      if (prev_history_pos != history_pos_) {
        const char* history_str =
            (history_pos_ >= 0) ? history_[history_pos_].c_str() : "";
        data->DeleteChars(0, data->BufTextLen);
        data->InsertChars(0, history_str);
      }
    }
  }
  return 0;
}

void Console::Draw(bool* p_open, vamiga::VAmiga& emu) {
  if (!p_open || !*p_open) return;

  ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Console", p_open)) {
    ImGui::End();
    return;
  }

  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Close Console")) *p_open = false;
    ImGui::EndPopup();
  }

  if (ImGui::Button("Clear")) {
    items_.clear();
  }
  ImGui::SameLine();
  if (ImGui::Button("Copy")) {
    ImGui::LogToClipboard();
  }
  ImGui::SameLine();

  static ImGuiTextFilter filter;
  filter.Draw("Filter (\"incl,-excl\")", 180);
  bool has_filter = filter.IsActive();
  ImGui::Separator();

  // Direct access to emulator RetroShell text buffer
  const char* text_ptr = emu.retroShell.text();
  std::string_view current_retro_shell_text = text_ptr ? text_ptr : "";

  if (current_retro_shell_text != retro_shell_current_text_) {
    size_t old_len = retro_shell_current_text_.length();
    if (current_retro_shell_text.length() > old_len) {
      std::string_view new_output = current_retro_shell_text.substr(old_len);

      size_t start = 0;
      while (start < new_output.length()) {
        size_t end = new_output.find('\n', start);
        if (end == std::string_view::npos) end = new_output.length();

        std::string_view line = new_output.substr(start, end - start);
        if (!line.empty()) {
          AddLog("{}", line);
        }
        start = end + 1;
      }
    }
    retro_shell_current_text_ = std::string(current_retro_shell_text);
  }

  const float footer_height_to_reserve =
      ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
  ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve),
                    false, ImGuiWindowFlags_HorizontalScrollbar);

  if (ImGui::BeginPopupContextWindow()) {
    if (ImGui::Selectable("Clear")) items_.clear();
    ImGui::EndPopup();
  }

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
  for (const auto& item : items_) {
    if (has_filter && !filter.PassFilter(item.c_str())) continue;

    bool has_color = false;
    if (item.find("[error]") != std::string::npos) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
      has_color = true;
    } else if (item.starts_with("# ")) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f));
      has_color = true;
    }

    ImGui::TextUnformatted(item.c_str());
    if (has_color) ImGui::PopStyleColor();
  }
  ImGui::PopStyleVar();

  if (scroll_to_bottom_ || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
    ImGui::SetScrollHereY(1.0f);
  scroll_to_bottom_ = false;

  ImGui::EndChild();
  ImGui::Separator();

  bool reclaim_focus = false;
  ImGuiInputTextFlags input_text_flags =
      ImGuiInputTextFlags_EnterReturnsTrue |
      ImGuiInputTextFlags_CallbackCompletion |
      ImGuiInputTextFlags_CallbackHistory;
  if (ImGui::InputText("Input", input_buf_.data(), input_buf_.size(),
                       input_text_flags, &TextEditCallbackStub, (void*)this)) {
    std::string_view sv(input_buf_.data());
    auto start = sv.find_first_not_of(" \t\n\r");
    if (start != std::string_view::npos) {
      auto end = sv.find_last_not_of(" \t\n\r");
      sv = sv.substr(start, end - start + 1);
      ExecCommand(sv);
    }

    std::fill(input_buf_.begin(), input_buf_.end(), 0);
    reclaim_focus = true;
  }

  ImGui::SetItemDefaultFocus();
  if (reclaim_focus) ImGui::SetKeyboardFocusHere(-1);

  ImGui::End();
}

}  // namespace gui
