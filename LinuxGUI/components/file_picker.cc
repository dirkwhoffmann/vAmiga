#include "file_picker.h"

#include <iostream>
#include <limits>

#ifdef __linux__
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace gui {

FilePicker& FilePicker::Instance() {
  static FilePicker instance;
  return instance;
}

FilePicker::FilePicker() { SetupBookmarks(); }

void FilePicker::SetupBookmarks() {}

void FilePicker::Open(const std::string& id, const PickerOptions& options,
                      std::function<void(std::filesystem::path)> on_select) {
  IGFD::FileDialogConfig config;
  config.path = options.initial_path.empty()
                    ? std::filesystem::current_path().string()
                    : options.initial_path.string();

  config.flags = ImGuiFileDialogFlags_DisableCreateDirectoryButton |
                 ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering;

  if (options.modal) config.flags |= ImGuiFileDialogFlags_Modal;

  std::string filters_str = options.filters;

  switch (options.mode) {
    case PickerMode::kSaveFile:
      config.flags |= ImGuiFileDialogFlags_ConfirmOverwrite;
      if (config.fileName.empty()) config.fileName = "untitled";
      break;
    case PickerMode::kOpenDirectory:
      filters_str = "";
      break;
    case PickerMode::kSaveDirectory:
      filters_str = "";
      config.flags |= ImGuiFileDialogFlags_ConfirmOverwrite;
      if (config.fileName.empty()) config.fileName = "NewFolder";
      break;
    default:
      break;
  }

  active_requests_[id] = {on_select, id};

  ImGuiFileDialog::Instance()->OpenDialog(id, options.title,
                                          filters_str.c_str(), config);
}

void FilePicker::Draw() {
  std::vector<std::string> to_remove;

  for (auto& [id, req] : active_requests_) {
    ImVec2 min_size(600, 400);
    ImVec2 max_size(std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::max());

    if (ImGuiFileDialog::Instance()->Display(id, ImGuiWindowFlags_NoCollapse,
                                             min_size, max_size)) {
      if (ImGuiFileDialog::Instance()->IsOk()) {
        std::string file_path_name =
            ImGuiFileDialog::Instance()->GetFilePathName();

        if (!file_path_name.empty()) {
          req.callback(std::filesystem::path(file_path_name));
        }
      }

      ImGuiFileDialog::Instance()->Close();
      to_remove.push_back(id);
    }
  }

  for (const auto& id : to_remove) {
    active_requests_.erase(id);
  }
}

}  // namespace gui
