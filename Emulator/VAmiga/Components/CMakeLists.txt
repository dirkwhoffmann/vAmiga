target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

Amiga.cpp
Emulator.cpp

)

add_subdirectory(Agnus)
add_subdirectory(CPU)
add_subdirectory(CIA)
add_subdirectory(Denise)
add_subdirectory(Memory)
add_subdirectory(Paula)
add_subdirectory(Ports)
add_subdirectory(RTC)
add_subdirectory(Zorro)
