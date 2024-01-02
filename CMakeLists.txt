cmake_minimum_required(VERSION 3.19)

project(engine VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(ENABLE_PRECOMPILED_HEADERS OFF)
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_libpng TRUE)
set(CMAKE_MACOSX_RPATH 1)
SET(CMAKE_INSTALL_RPATH @loader_path/../Framework)
set(BUILD_SHARED_LIBS ON)
set(GLFW_INSTALL ON)

# set(GLFW_LIBRARY_TYPE SHARED)

# add_executable(${PROJECT_NAME} MACOSX_BUNDLE src/main.cpp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/freetype2)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/cereal)

set(FMOD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod/api")
set(FMOD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod/api/studio")
set(FMOD_CORE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod/api/core")

find_library((fmod))

add_library(fmod SHARED IMPORTED GLOBAL)
set_property(TARGET fmod PROPERTY
    IMPORTED_LOCATION "${FMOD_DIR}/core/lib/libfmodL.dylib")
set_property(TARGET fmod PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_DIR}/core/inc")

add_library(fmod_studio SHARED IMPORTED GLOBAL)
set_property(TARGET fmod_studio PROPERTY
    IMPORTED_LOCATION "${FMOD_DIR}/studio/lib/libfmodstudioL.dylib")
set_property(TARGET fmod_studio PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${FMOD_DIR}/studio/inc")

# find_package(fmod)
include_directories("${PROJECT_BINARY_DIR}" glfw/include glad/include freetype2/include glm/include stb_image/ "${FMOD_DIR}/core/inc" "${FMOD_DIR}/studio/inc" "${FMOD_DIR}/core/lib" "${FMOD_DIR}/studio/lib")
add_library(${PROJECT_NAME} SHARED src/)

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# add property for shaders folder - to be copied into game build "Assets/" folder
set_property(
    TARGET ${PROJECT_NAME}
    APPEND PROPERTY
    SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)

# set_target_properties(${PROJECT_NAME}
# PROPERTIES SUFFIX ".html"
# LINK_FLAGS " -O3 -sFULL_ES3 -sUSE_GLFW=3 -sUSE_FREETYPE=1 -s EXPORTED_RUNTIME_METHODS=['cwrap','setValue','getValue'] --preload-file Assets")
# target_include_directories(${PROJECT_NAME} INTERFACE src/)
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" src/ vendor/glfw/include vendor/glad/include vendor/freetype2/include vendor/glm vendor/stb_image/ "${FMOD_DIR}/core/inc" "${FMOD_DIR}/studio/inc" "${FMOD_DIR}/core/lib" "${FMOD_DIR}/studio/lib" vendor/cereal/include)

# target_include_directories(${PROJECT_NAME} INTERFACE src/)
target_link_libraries(${PROJECT_NAME} fmod fmod_studio glad glfw glm freetype)
link_libraries(${PROJECT_NAME} fmod fmod_studio})

install(TARGETS ${PROJECT_NAME}
    glad freetype glm glfw DESTINATION ${PROJECT_BINARY_DIR}
    BUNDLE DESTINATION ${PROJECT_BINARY_DIR}
)
install(IMPORTED_RUNTIME_ARTIFACTS ${PROJECT_NAME})

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${FMOD_DIR}/core/lib/libfmodL.dylib
    ${CMAKE_CURRENT_BINARY_DIR}/libfmodL.dylib)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${FMOD_DIR}/studio/lib/libfmodstudioL.dylib
    ${CMAKE_CURRENT_BINARY_DIR}/libfmodstudioL.dylib)

# target_link_libraries(${PROJECT_NAME} glad glfw glm freetype "${FMOD_DIR}/studio/lib/upstream/w32/fmodstudio_wasm.a" "${FMOD_DIR}/core/lib/upstream/w32/fmod_wasm.a")
# add_custom_command(
# TARGET ${PROJECT_NAME} POST_BUILD
# COMMAND ${CMAKE_COMMAND} -E copy_directory
# ${CMAKE_SOURCE_DIR}/shaders
# ${CMAKE_CURRENT_BINARY_DIR}/shaders)

# add_custom_command(
# TARGET ${PROJECT_NAME} POST_BUILD
# COMMAND ${CMAKE_COMMAND} -E copy_directory
# ${CMAKE_SOURCE_DIR}/fonts
# ${CMAKE_CURRENT_BINARY_DIR}/fonts)

# add_custom_command(
# TARGET ${PROJECT_NAME} POST_BUILD
# COMMAND ${CMAKE_COMMAND} -E copy_directory
# ${CMAKE_SOURCE_DIR}/textures
# ${CMAKE_CURRENT_BINARY_DIR}/textures)