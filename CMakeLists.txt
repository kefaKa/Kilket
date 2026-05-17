cmake_minimum_required(VERSION 3.16)

project(Kilket VERSION 0.1.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
configure_file(src/version.h.in version.h)
set(CMAKE_CXX_STANDARD 20)

option(BUILD_TESTING "Build the test suite" ON)

# core library
add_library(kilket_lib
    src/filewatcher.cpp
    src/session_logger.cpp
    src/task_runner.cpp
    src/kilket_core.cpp
    src/config_manager.cpp
)

target_include_directories(kilket_lib PUBLIC src/include)
target_include_directories(kilket_lib PUBLIC src/include ${CMAKE_CURRENT_BINARY_DIR})

# main executables
add_executable(kilket_core src/main.cpp)
target_link_libraries(kilket_core PRIVATE kilket_lib)
