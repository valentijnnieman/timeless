cmake_minimum_required(VERSION 3.29)

project(timeless VERSION 0.3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(ENABLE_PRECOMPILED_HEADERS OFF)
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz TRUE)
set(CMAKE_DISABLE_FIND_PACKAGE_libpng TRUE)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_DOC OFF)
set(BUILD_SANDBOX OFF)
# set(GLFW_INSTALL ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

option(EMSCRIPTEN "WASM build or regular" OFF)

add_library(${PROJECT_NAME} STATIC "example/main.cpp")
  
# add property for shaders folder - to be copied into game build "Assets/" folder
  set_property(
      TARGET ${PROJECT_NAME}
      APPEND PROPERTY
      SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
  )

if(EMSCRIPTEN)
  message(STATUS "EMSCIRPTEN BUILD!")

  set(CMAKE_EXECUTABLE_SUFFIX ".html")

  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)

  set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
  target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" src/ vendor/freetype2/include vendor/glm vendor/stb_image/)

  target_link_options(${PROJECT_NAME} PUBLIC -sASYNCIFY -sEXTRA_EXPORTED_RUNTIME_METHODS=['cwrap','setValue','getValue'] -sUSE_FREETYPE=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3 -sALLOW_MEMORY_GROWTH -sSTB_IMAGE=1 --embed-file Assets)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --use-port=contrib.glfw3")

  target_link_libraries(${PROJECT_NAME} glm freetype)

  add_subdirectory(example example)

else()
  if(apple)
      # set(CMAKE_MACOSX_RPATH 1)
      # SET(CMAKE_INSTALL_RPATH @loader_path/../Framework)
  endif()

  if(LINUX)
    set(GLFW_BUILD_WAYLAND OFF)
    set(GLFW_BUILD_X11 ON)
  endif()

# set(GLFW_LIBRARY_TYPE SHARED)

# add_executable(${PROJECT_NAME} MACOSX_BUNDLE src/main.cpp)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/freetype2)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/cereal)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/openal-soft)

  include_directories("${PROJECT_BINARY_DIR}" glad/include glfw/include freetype2/include glm/include stb_image/)

  set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# set_target_properties(${PROJECT_NAME}
# PROPERTIES SUFFIX ".html"
# LINK_FLAGS " -O3 -sFULL_ES3 -sUSE_GLFW=3 -sUSE_FREETYPE=1 -s EXPORTED_RUNTIME_METHODS=['cwrap','setValue','getValue'] --preload-file Assets")
# target_include_directories(${PROJECT_NAME} INTERFACE src/)
  target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" src/ vendor/glfw/include vendor/glad/include vendor/freetype2/include vendor/glm vendor/stb_image/ vendor/cereal/include)

# target_include_directories(${PROJECT_NAME} INTERFACE src/)
target_link_libraries(${PROJECT_NAME} glad glfw glm freetype OpenAL::OpenAL)
  add_subdirectory(example example)

endif()
