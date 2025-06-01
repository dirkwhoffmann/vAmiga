target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

DriveDescriptors.cpp
Drive.cpp
FloppyDrive.cpp
HardDrive.cpp
FloppyDisk.cpp

)
