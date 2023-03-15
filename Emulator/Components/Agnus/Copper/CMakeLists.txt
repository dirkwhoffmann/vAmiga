target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

Copper.cpp
CopperInfo.cpp
CopperRegs.cpp
CopperEvents.cpp
CopperDebugger.cpp

)
