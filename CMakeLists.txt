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


if(BUILD_TESTING)
    # tests
    add_executable(test_filewatcher tests/unit/test_filewatcher.cpp)
    target_link_libraries(test_filewatcher PRIVATE kilket_lib)
    target_include_directories(test_filewatcher PRIVATE tests)

    add_executable(test_task_runner tests/unit/test_task_runner.cpp)
    target_link_libraries(test_task_runner PRIVATE kilket_lib)
    target_include_directories(test_task_runner PRIVATE tests)

    add_executable(test_session_logger tests/unit/test_session_logger.cpp)
    target_link_libraries(test_session_logger PRIVATE kilket_lib)
    target_include_directories(test_session_logger PRIVATE tests)

    add_executable(test_config_manager tests/unit/test_config_manager.cpp)
    target_link_libraries(test_config_manager PRIVATE kilket_lib)
    target_include_directories(test_config_manager PRIVATE tests)
    target_include_directories(test_config_manager PUBLIC tests/unit ${CMAKE_CURRENT_BINARY_DIR})
    target_compile_definitions(test_config_manager PRIVATE KILKET_TESTING)

    add_executable(test_kilket_core tests/integration/test_kilket_core.cpp)
    target_link_libraries(test_kilket_core PRIVATE kilket_lib)
    target_include_directories(test_kilket_core PRIVATE tests)

    enable_testing()
    add_test(NAME kilket_filewatcher_tests COMMAND test_filewatcher)
    add_test(NAME kilket_task_runner_tests COMMAND test_task_runner)
    add_test(NAME kilket_session_logger_tests COMMAND test_session_logger)
    add_test(NAME kilket_config_manager_tests COMMAND test_config_manager)
    add_test(NAME kilket_core_tests COMMAND test_kilket_core)
endif()

add_executable(kilket cli/src/main.cpp
                            cli/src/commands/add.cpp
                            cli/src/commands/run.cpp
                            cli/src/commands/init.cpp
                            cli/src/commands/list.cpp
                            cli/src/commands/remove.cpp
                            cli/src/commands/set.cpp
                            cli/src/commands/check.cpp)
target_link_libraries(kilket PRIVATE kilket_lib)
target_include_directories(kilket PRIVATE cli/src)
target_include_directories(kilket PRIVATE cli/src ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS kilket DESTINATION bin)
# --
