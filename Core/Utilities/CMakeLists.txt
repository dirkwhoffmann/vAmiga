target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

  Compression.cpp
  Checksum.cpp
  StringUtils.cpp
  IOUtils.cpp
  Parser.cpp

)
