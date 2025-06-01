target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

Emulator.cpp

)

add_subdirectory(Foundation)
add_subdirectory(Components)
add_subdirectory(FileSystems)
add_subdirectory(Media)
add_subdirectory(Misc)
add_subdirectory(Peripherals)
add_subdirectory(Ports)
add_subdirectory(ThirdParty)
add_subdirectory(Utilities)
