add_executable(simple-test-harness main.cpp)
target_link_libraries(simple-test-harness EchoServer)

add_test(NAME SimpleTests COMMAND simple-test-harness)
