target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

Console.cpp
CommandConsole.cpp
DebugConsole.cpp
RetroShell.cpp
RetroShellCmd.cpp
TextStorage.cpp

)
