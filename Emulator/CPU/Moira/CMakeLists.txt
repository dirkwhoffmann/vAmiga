target_sources(vAmigaCore PRIVATE

Moira.cpp
MoiraDebugger.cpp
softfloat/softfloat.cpp
)

target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/softfloat)
