target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

               AmigaEncoder.cpp
               DiskEncoder.cpp
               GCR.cpp
               IBMEncoder.cpp
               MFM.cpp

               )
