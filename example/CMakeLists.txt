cmake_minimum_required(VERSION 3.19)

project(example)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(ENABLE_PRECOMPILED_HEADERS OFF)

add_executable(${PROJECT_NAME} main.cpp)

get_target_property(ENGINE_INCLUDES timeless INCLUDE_DIRECTORIES)
get_target_property(SHADERS_DIR timeless SHADERS_DIR)
get_target_property(FMOD_DIR timeless FMOD_DIR)

# Construct the compiler string for the include directories.
foreach(dir ${ENGINE_INCLUDES})
    string(APPEND INCLUDE_COMPILER_STRING "/I${dir} ")
endforeach()

# target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" ${INCLUDE_COMPILER_STRING})
target_link_libraries(${PROJECT_NAME} PUBLIC timeless)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/Assets
    ${CMAKE_CURRENT_BINARY_DIR}/Assets)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHADERS_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/Assets/shaders)

