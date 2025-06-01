target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

)

add_subdirectory(Drive)
add_subdirectory(Mouse)
add_subdirectory(Joystick)
add_subdirectory(Keyboard)
add_subdirectory(Monitor)
