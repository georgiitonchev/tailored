cmake_minimum_required(VERSION 3.10)

project(tailored)

# Add all source files in the specified directory
file(GLOB_RECURSE SOURCE_FILES "src/*.c" "src/*/*.c" "src/*/*/*.c")

add_executable(tailored ${SOURCE_FILES})

# Copy resources preserving directory structure
file(COPY "res" DESTINATION "${CMAKE_BINARY_DIR}")