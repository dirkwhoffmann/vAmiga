target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

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
