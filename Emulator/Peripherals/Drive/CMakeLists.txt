target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

DriveDescriptors.cpp
Drive.cpp
FloppyDrive.cpp
HardDrive.cpp
FloppyDisk.cpp

)
