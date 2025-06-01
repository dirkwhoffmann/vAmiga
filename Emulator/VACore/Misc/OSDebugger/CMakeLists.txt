target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

OSDescriptors.cpp
OSDebugger.cpp
OSDebuggerRead.cpp
OSDebuggerDump.cpp

)
