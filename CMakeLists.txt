cmake_minimum_required(VERSION 3.13)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(WebServer main.cpp Server.cpp Server.h Log.cpp Log.h Worker.cpp Worker.h ShareMemory.cpp ShareMemory.h Lock.cpp Lock.h MemoryPool.cpp MemoryPool.h Array.cpp Array.h FastCGI.cpp FastCGI.h CGIManager.cpp CGIManager.h Timer.cpp Timer.h HTTPParse.cpp HTTPParse.h FileManager.cpp FileManager.h ThreadPool.cpp ThreadPool.h Setting.cpp Setting.h)

#add_compile_options(-pthread)

target_link_libraries(WebServer pthread)