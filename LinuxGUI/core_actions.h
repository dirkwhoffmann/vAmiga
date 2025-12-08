#ifndef LINUXGUI_CORE_ACTIONS_H_
#define LINUXGUI_CORE_ACTIONS_H_

#include <string>
#include <string_view>

#include "Misc/RetroShell/RetroShellTypes.h"

namespace core_actions {
    std::string_view GetRetroShellText();
    void RetroShellPressKey(vamiga::RSKey key, bool shift);
    void RetroShellPressChar(char c);
    void RetroShellPressString(const std::string& s);
}  // namespace core_actions

#endif  // LINUXGUI_CORE_ACTIONS_H_
