target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

Console.cpp
CommandConsole.cpp
DebugConsole.cpp
RetroShell.cpp
RetroShellCmd.cpp
TextStorage.cpp

)
