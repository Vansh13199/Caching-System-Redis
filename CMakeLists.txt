cmake_minimum_required(VERSION 3.10)
project(RedisClone VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Platform specific configurations
if(WIN32)
    # Windows specific libraries for sockets
    link_libraries(ws2_32)
else()
    # Unix specific libraries for threading
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    link_libraries(Threads::Threads)
endif()

add_executable(redis_clone 
    src/main.cpp 
    src/server.cpp 
    src/storage.cpp
)
