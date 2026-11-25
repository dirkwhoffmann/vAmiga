target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

Moira.cpp
MoiraCore68000.cpp
MoiraCore68010.cpp
MoiraCore68020.cpp
MoiraDebugger.cpp
Cache/Cache.cpp
FPU/MFloat80.cpp
FPU/FpuEnv.cpp
FPU/FpuEnvMath.cpp
FPU/FpuFormats.cpp
FPU/FPU.cpp
MMU/MMU030.cpp
MMU/MMU040.cpp
)
