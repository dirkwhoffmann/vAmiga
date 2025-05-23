target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

AudioPort.cpp
ControlPort.cpp
SerialPort.cpp
VideoPort.cpp
SampleRateDetector.cpp

)
