target_sources(vAmiga PRIVATE

Paula.cpp
PaulaRegisters.cpp
PaulaEvents.cpp

)

add_subdirectory(Audio)
add_subdirectory(DiskController)
add_subdirectory(UART)
