target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

Agnus.cpp
AgnusInfo.cpp
AgnusRegs.cpp
AgnusDma.cpp
AgnusEvents.cpp
Beam.cpp

)

add_subdirectory(Sequencer)
add_subdirectory(Copper)
add_subdirectory(Blitter)
add_subdirectory(DmaDebugger)
