target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

Console.cpp
CommanderConsole.cpp
DebuggerConsole.cpp
NavigatorConsole.cpp
RetroShell.cpp
RSCommand.cpp
TextStorage.cpp

)
