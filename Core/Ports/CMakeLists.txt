target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

AudioPort.cpp
ControlPort.cpp
SerialPort.cpp
VideoPort.cpp
SampleRateDetector.cpp

)
