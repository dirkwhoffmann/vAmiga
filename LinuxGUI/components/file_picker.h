#ifndef LINUXGUI_COMPONENTS_FILE_PICKER_H_
#define LINUXGUI_COMPONENTS_FILE_PICKER_H_

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "ImGuiFileDialog.h"

namespace gui {

enum class PickerMode {
  kOpenFile,
  kSaveFile,
  kOpenDirectory,
  kSaveDirectory
};

struct PickerOptions {
  std::string title;
  std::string filters;
  std::filesystem::path initial_path;
  PickerMode mode = PickerMode::kOpenFile;
  bool modal = true;
};

class FilePicker {
 public:
  static FilePicker& Instance();

  FilePicker(const FilePicker&) = delete;
  FilePicker& operator=(const FilePicker&) = delete;

  void Open(const std::string& id, const PickerOptions& options,
            std::function<void(std::filesystem::path)> on_select);

  void Draw();

 private:
  FilePicker();

  struct ActiveRequest {
    std::function<void(std::filesystem::path)> callback;
    std::string id;
  };

  std::map<std::string, ActiveRequest> active_requests_;

  void SetupBookmarks();
};

}  // namespace gui

#endif  // LINUXGUI_COMPONENTS_FILE_PICKER_H_
