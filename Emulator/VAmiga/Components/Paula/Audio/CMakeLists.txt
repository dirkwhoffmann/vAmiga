target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

StateMachine.cpp
StateMachineRegs.cpp
StateMachineEvents.cpp
Sampler.cpp
AudioFilter.cpp
AudioStream.cpp

)
