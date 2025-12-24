add_library(EchoServer SocketServer.cpp SocketClient.cpp Logger.cpp)
target_include_directories(EchoServer PUBLIC ${PROJECT_SOURCE_DIR}/include)
