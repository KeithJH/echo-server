add_library(EchoServer SocketServer.cpp SocketClient.cpp util.cpp)
target_include_directories(EchoServer PUBLIC ${PROJECT_SOURCE_DIR}/include)
