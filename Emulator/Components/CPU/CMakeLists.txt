target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

CPU.cpp
CPUDebugger.cpp

)

add_subdirectory(Moira)
