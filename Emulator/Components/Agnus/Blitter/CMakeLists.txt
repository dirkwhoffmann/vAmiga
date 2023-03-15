target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

Blitter.cpp
BlitterInfo.cpp
BlitterRegs.cpp
BlitterEvents.cpp
FastBlitter.cpp
SlowBlitter.cpp

)
