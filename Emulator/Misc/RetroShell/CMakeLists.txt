target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

Command.cpp
Interpreter.cpp
InterpreterCmds.cpp
InterpreterDebugCmds.cpp
RetroShell.cpp
TextStorage.cpp

)
