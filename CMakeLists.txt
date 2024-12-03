# Minimum CMake version required
cmake_minimum_required(VERSION 3.0)

# Project name and language
project(the_legend_of_alberta)
set(CMAKE_CXX_STANDARD 14)

# Set build type to Debug by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Include Raylib headers
include_directories(/opt/homebrew/Cellar/raylib/5.5/include)

# Link Raylib library directory
link_directories(/opt/homebrew/lib)

## Add source files (automatically finds all .cpp files in src/)
file(GLOB_RECURSE the_legend_of_alberta_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.*")

## Add the src/ directory for header files
#target_include_directories(the_legend_of_alberta PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/*.h")

# Add executable
add_executable(${PROJECT_NAME} ${the_legend_of_alberta_SOURCES})

# Link Raylib and additional libraries
target_link_libraries(${PROJECT_NAME} PRIVATE raylib m pthread dl)

# Set the assets path macro
target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

# Enable verbose compiler output for debugging
set(CMAKE_VERBOSE_MAKEFILE ON)

# Output the executable in the build directory
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
