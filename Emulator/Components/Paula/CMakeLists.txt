target_sources(vAmigaCore PRIVATE

Paula.cpp
PaulaRegs.cpp
PaulaEvents.cpp

)

add_subdirectory(Audio)
add_subdirectory(DiskController)
add_subdirectory(UART)
