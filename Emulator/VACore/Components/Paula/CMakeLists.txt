target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

Paula.cpp
PaulaRegs.cpp
PaulaEvents.cpp

)

add_subdirectory(Audio)
add_subdirectory(DiskController)
add_subdirectory(UART)

