target_include_directories(vAmigaCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(vAmigaCore PRIVATE

RemoteManager.cpp
RemoteServer.cpp
SocketServer.cpp
HttpServer.cpp
SerServer.cpp
RshServer.cpp
GdbServer.cpp
GdbServerCmds.cpp
PromServer.cpp
Socket.cpp

)
