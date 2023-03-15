target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

ZorroManager.cpp
ZorroBoard.cpp
RamExpansion.cpp
HdController.cpp
DiagBoard.cpp

)
