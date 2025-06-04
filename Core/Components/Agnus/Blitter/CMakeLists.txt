target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

Blitter.cpp
BlitterInfo.cpp
BlitterRegs.cpp
BlitterEvents.cpp
FastBlitter.cpp
SlowBlitter.cpp

)
