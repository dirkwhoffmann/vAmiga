target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

Copper.cpp
CopperInfo.cpp
CopperRegs.cpp
CopperEvents.cpp
CopperDebugger.cpp

)
