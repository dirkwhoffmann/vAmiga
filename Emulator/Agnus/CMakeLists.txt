target_sources(vAmigaCore PRIVATE

Agnus.cpp
AgnusInfo.cpp
AgnusRegs.cpp
AgnusDma.cpp
AgnusEvents.cpp
Scheduler.cpp

)

add_subdirectory(Sequencer)
add_subdirectory(Copper)
add_subdirectory(Blitter)
add_subdirectory(DmaDebugger)
