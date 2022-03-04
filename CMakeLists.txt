cmake_minimum_required (VERSION 3.14)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Define an executable target
project(dungeoneer)

find_package(OpenGL REQUIRED)
add_executable(dungeoneer WIN32)

# Specify the required C standard
set_target_properties(dungeoneer PROPERTIES C_STANDARD 17)
set_target_properties(dungeoneer PROPERTIES CMAKE_C_STANDARD_REQUIRED True)

add_compile_definitions(BUILD_WIN32)

# Add all of the source code for the exe itself
add_subdirectory(game/source/)

