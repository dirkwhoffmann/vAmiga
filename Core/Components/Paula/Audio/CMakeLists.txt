target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

StateMachine.cpp
StateMachineRegs.cpp
StateMachineEvents.cpp
Sampler.cpp
AudioFilter.cpp
AudioStream.cpp

)
