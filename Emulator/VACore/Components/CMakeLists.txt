target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

Amiga.cpp

)

add_subdirectory(Agnus)
add_subdirectory(CPU)
add_subdirectory(CIA)
add_subdirectory(Denise)
add_subdirectory(Memory)
add_subdirectory(Paula)
add_subdirectory(RTC)
add_subdirectory(Zorro)
