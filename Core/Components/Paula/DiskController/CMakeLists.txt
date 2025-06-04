target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

DiskController.cpp
DiskControllerRegs.cpp
DiskControllerEvents.cpp

)
