cmake_minimum_required(VERSION 3.19)

project(engine VERSION 0.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(ENABLE_PRECOMPILED_HEADERS OFF)
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_libpng TRUE)
set(BUILD_SHARED_LIBS OFF)
set(GLFW_INSTALL ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(apple)
    set(CMAKE_MACOSX_RPATH 1)
    SET(CMAKE_INSTALL_RPATH @loader_path/../Framework)
endif()

# set(GLFW_LIBRARY_TYPE SHARED)

# add_executable(${PROJECT_NAME} MACOSX_BUNDLE src/main.cpp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/freetype2)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/cereal)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod)

# find_package(fmod)
include_directories("${PROJECT_BINARY_DIR}" glad/include glfw/include freetype2/include glm/include stb_image/)
add_library(${PROJECT_NAME} SHARED "src/test.cpp")

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# add property for shaders folder - to be copied into game build "Assets/" folder
set_property(
    TARGET ${PROJECT_NAME}
    APPEND PROPERTY
    SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)

# add property for fmod folder to copy dll files
set_property(
    TARGET ${PROJECT_NAME}
    APPEND PROPERTY
    FMOD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod"
)

# set_target_properties(${PROJECT_NAME}
# PROPERTIES SUFFIX ".html"
# LINK_FLAGS " -O3 -sFULL_ES3 -sUSE_GLFW=3 -sUSE_FREETYPE=1 -s EXPORTED_RUNTIME_METHODS=['cwrap','setValue','getValue'] --preload-file Assets")
# target_include_directories(${PROJECT_NAME} INTERFACE src/)
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" src/ vendor/glfw/include vendor/glad/include vendor/freetype2/include vendor/glm vendor/stb_image/ vendor/cereal/include)

# target_include_directories(${PROJECT_NAME} INTERFACE src/)
target_link_libraries(${PROJECT_NAME} glad glfw glm freetype fmod fmod_studio)
link_libraries(${PROJECT_NAME} fmod fmod_studio)

if(WIN32)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod/win/core/lib/fmod.dll
        ${CMAKE_CURRENT_BINARY_DIR}/fmod.dll)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod/win/core/lib/fmod_vc.lib
        ${CMAKE_CURRENT_BINARY_DIR}/fmod_vc.lib)

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod/win/studio/lib/fmodstudio.dll
        ${CMAKE_CURRENT_BINARY_DIR}/fmodstudio.dll)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod/win/studio/lib/fmodstudio_vc.lib
        ${CMAKE_CURRENT_BINARY_DIR}/fmodstudio_vc.lib)
endif()

if(apple)
    install(TARGETS ${PROJECT_NAME}
        glad freetype glm glfw DESTINATION ${PROJECT_BINARY_DIR}
        BUNDLE DESTINATION ${PROJECT_BINARY_DIR}
    )
    install(IMPORTED_RUNTIME_ARTIFACTS ${PROJECT_NAME})

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${FMOD_DIR}/apple/core/lib/libfmodL.dylib
        ${CMAKE_CURRENT_BINARY_DIR}/libfmodL.dylib)

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${FMOD_DIR}/apple/studio/lib/libfmodstudioL.dylib
        ${CMAKE_CURRENT_BINARY_DIR}/libfmodstudioL.dylib)
endif()

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