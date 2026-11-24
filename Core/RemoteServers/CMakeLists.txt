target_include_directories(VACore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(VACore PRIVATE

RemoteManager.cpp
RemoteServer.cpp
Transport.cpp
StdioTransport.cpp
TcpTransport.cpp
HttpTransport.cpp
SerServer.cpp
RshServer.cpp
RpcServer.cpp
GdbServer.cpp
GdbServerCmds.cpp
PromServer.cpp
Socket.cpp

)
