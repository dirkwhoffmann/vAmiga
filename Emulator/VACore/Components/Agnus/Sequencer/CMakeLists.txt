target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

Sequencer.cpp
SequencerDas.cpp
SequencerBpl.cpp
SequencerInfo.cpp
SequencerRegs.cpp

)
