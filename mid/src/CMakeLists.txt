cmake_minimum_required(VERSION 3.26)
project(mid CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 20)

find_package(nlohmann_json REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZEROMQ REQUIRED libzmq)
pkg_check_modules(UUID REQUIRED uuid)

add_executable(mid)
target_sources(mid PRIVATE main.cpp PRIVATE signal_handler.cpp PRIVATE utils.cpp PRIVATE req.cpp PRIVATE sub.cpp)

target_link_libraries(mid nlohmann_json::nlohmann_json)
target_link_libraries(mid ${ZEROMQ_LIBRARIES})
target_link_libraries(mid ${UUID_LIBRARIES})
