target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

Colors.cpp
Denise.cpp
DeniseDebugger.cpp
DeniseInfo.cpp
DeniseRegs.cpp
FrameBuffer.cpp
PixelEngine.cpp

)
