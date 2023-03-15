target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

CIA.cpp
CIARegs.cpp
CIAEvents.cpp
TOD.cpp

)
