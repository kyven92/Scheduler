cmake_minimum_required(VERSION 3.10)
project(TaskGraphExample)

# Add the source files
set(SOURCE_FILES
        Scheduler.cpp
        Test.cpp)

# Add an executable for the main program
add_executable(TaskGraphExample ${SOURCE_FILES})

# Set C++ standard
set_target_properties(TaskGraphExample PROPERTIES CXX_STANDARD 14)

# Optionally enable compiler warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(TaskGraphExample PRIVATE -Wall -Wextra -Wpedantic)
elseif (MSVC)
    target_compile_options(TaskGraphExample PRIVATE /W4)
endif()

# Add a target for running the test
add_test(NAME TaskGraphExampleTest COMMAND TaskGraphExample)

# Print a message with instructions after successful build
message(STATUS "Build complete. To run the test, use: ctest")
