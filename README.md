# echo-server
## Goal
Explore and demonstrate creating a simple "echo server" with sockets and
edge-triggered polling. Connecting to the server over unix sockets or IPv4
sends back any content written to the server. The server can handle multiple
clients with work distributed over a number of worker threads, though the
number of clients can be larger than the number of worker threads.

## Dependencies
Currently does not have any external dependencies.

## Building
Built using CMake, with optional presets for `debug` and `release`. Only tested
on Linux with `g++` and `clang++`.
```
$ cmake --list-presets
Available configure presets:

  "debug"   - debug
  "release" - release
```

Building with a preset configuration:
```
$ cmake --preset debug
$ cmake --build build/debug/
```

Building with a manual configuration:
```
$ mkdir mybuild
$ cmake -S. -Bmybuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++
$ cmake --build mybuild
```

## Running
The created `echo-server` binary expects two arguments. The first is either
"inet" or "unix", followed by either a port number or file path, respectively.

To start a server and connect over standard IPv4 TCP:
```
$ ./build/debug/src/EchoServerCli/echo-server inet 9090 &
$ nc -Nv localhost 9090
Connection to localhost (127.0.0.1) 9090 port [tcp/*] succeeded!
<type some message>
<same message is replied back>
EOF from client connection
```

To start a server and connect over Unix sockets:
```
$ ./build/debug/src/EchoServerCli/echo-server unix file.sock
$ nc -NU file.sock
<type some message>
<same message is replied back>
EOF from client connection
```
