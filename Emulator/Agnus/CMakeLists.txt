target_sources(vAmigaCore PRIVATE

DDF.cpp
Agnus.cpp
AgnusRegisters.cpp
AgnusDDF.cpp
AgnusDma.cpp
AgnusEvents.cpp
Scheduler.cpp
DmaDebugger.cpp

)

add_subdirectory(Copper)
add_subdirectory(Blitter)
