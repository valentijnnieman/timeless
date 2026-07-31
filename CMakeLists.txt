cmake_minimum_required(VERSION 3.19)

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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
set(ASSIMP_BUILD_ZLIB ON)

option(EMSCRIPTEN "WASM build or regular" OFF)
option(EMSCRIPTEN_PTHREADS "PThread support" OFF)

# ---------------------------------------------------------------------------
# Modular sub-targets
#
#   timeless        — core engine (components, rendering, input, window, ...)
#   timeless-audio  — FMOD-backed SoundSystem (the ONLY target that links FMOD)
#   timeless-ai     — NpcAiSystem time-sequencing (rewind/replay)
#
# The optional feature modules depend on core one-way; core never references
# them (verified: SoundSystem/NpcAiSystem are each referenced only by their own
# translation unit). So a game links just the modules it uses — e.g. BiasBusters
# links `timeless timeless-audio` and pays nothing for the AI module or, if it
# dropped audio too, for FMOD.
# ---------------------------------------------------------------------------

# Feature-module sources, carved out of the source tree.
set(TIMELESS_AUDIO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/timeless/systems/sound_system.cpp")
set(TIMELESS_AI_SOURCES    "${CMAKE_CURRENT_SOURCE_DIR}/src/timeless/systems/npc_ai_system.cpp")

# Everything else (minus the standalone resource packer) is core.
file(GLOB_RECURSE TIMELESS_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/timeless/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/timeless/*.cpp"
)
list(FILTER TIMELESS_CORE_SOURCES EXCLUDE REGEX ".*resource_packer\\.cpp$")
list(REMOVE_ITEM TIMELESS_CORE_SOURCES ${TIMELESS_AUDIO_SOURCES} ${TIMELESS_AI_SOURCES})

add_library(${PROJECT_NAME} STATIC ${TIMELESS_CORE_SOURCES})
add_library(${PROJECT_NAME}-audio STATIC ${TIMELESS_AUDIO_SOURCES})
add_library(${PROJECT_NAME}-ai STATIC ${TIMELESS_AI_SOURCES})

# Feature modules compile against core's headers/flags but core never links them
# back (keeps the dependency graph acyclic). PRIVATE so core's link options
# aren't re-propagated to consumers a second time through these libs.
target_link_libraries(${PROJECT_NAME}-audio PRIVATE ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}-ai    PRIVATE ${PROJECT_NAME})

# add property for shaders folder - to be copied into game build "Assets/" folder
set_property(
      TARGET ${PROJECT_NAME}
      APPEND PROPERTY
      SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
  )

if(EMSCRIPTEN)
  message(STATUS "EMSCIRPTEN BUILD!")
  # Threading must be CONSISTENT across every translation unit (engine, vendored
  # libs like Assimp/GLM, and the consuming game) and match the link mode.
  # Emscripten forbids mixing -pthread and non-pthread objects: they disagree on
  # the memory model (shared vs non-shared), atomics and TLS layout, which the
  # optimizer turns into miscompiled code at -O2/-O3. So only add -pthread when
  # actually building threaded; a single-threaded build must have it nowhere.
  if(EMSCRIPTEN_PTHREADS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
  endif()

  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp)

  set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
  target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" include/ src/ vendor/freetype2/include vendor/glm vendor/stb_image/)

  # The Emscripten SDL2 port must be activated at COMPILE time too: its bundled
  # <SDL.h> is a stub that errors unless -sUSE_SDL=2 is present while compiling
  # (link-time alone is not enough). PUBLIC so consumers compiling against our
  # headers (which pull in <SDL.h> transitively) get it as well.
  target_compile_options(${PROJECT_NAME} PUBLIC -sUSE_SDL=2)

  if(EMSCRIPTEN_PTHREADS)
    target_compile_options(${PROJECT_NAME} PUBLIC
      -pthread
    )
    target_link_options(${PROJECT_NAME} PUBLIC
      -sALLOW_MEMORY_GROWTH=1
      # With --shared-memory (pthreads) the ~20MB of --embed-file Assets pushes
      # static data past the default 16MB initial-memory floor, so raise it.
      # 64MB = ~21.5MB static + headroom for the runtime heap; growth covers the
      # rest. (Not needed in the single-threaded branch, which grows cheaply.)
      -sINITIAL_MEMORY=67108864
      -sEXPORTED_RUNTIME_METHODS=['cwrap','setValue','getValue']
      -sUSE_FREETYPE=1
      -sUSE_SDL=2
      # --use-port=contrib.glfw3
      -sMIN_WEBGL_VERSION=2
      -sMAX_WEBGL_VERSION=2
      -sFULL_ES3
      -sSTB_IMAGE=1
      --embed-file Assets
      -pthread
      -sPTHREAD_POOL_SIZE=5
    )
  else()
    target_link_options(${PROJECT_NAME} PUBLIC
      -sALLOW_MEMORY_GROWTH=1
      -sEXPORTED_RUNTIME_METHODS=['cwrap','setValue','getValue']
      -sUSE_FREETYPE=1
      -sUSE_SDL=2
      # --use-port=contrib.glfw3
      -sMIN_WEBGL_VERSION=2
      -sMAX_WEBGL_VERSION=2
      -sFULL_ES3
      -sSTB_IMAGE=1
      --embed-file Assets
    )
  endif()

  # Default web shell (canvas scaling, fullscreen, audio start-gate) for every
  # game that links timeless. PUBLIC so it propagates to the consuming
  # executable's link; a game can override it by adding its own --shell-file
  # later (the last --shell-file wins). See web/README.md.
  target_link_options(${PROJECT_NAME} PUBLIC
    --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)

  target_link_libraries(${PROJECT_NAME} glm assimp)
  # FMOD lives only in the audio module.
  target_link_libraries(${PROJECT_NAME}-audio PUBLIC fmod fmod_studio)

  # add_subdirectory(example example)

else()
  if(apple)
    # set(CMAKE_MACOSX_RPATH 1)
    # SET(CMAKE_INSTALL_RPATH @loader_path/../Framework)
  endif()

  # SDL2 (vendored) replaces GLFW for windowing/input. Build the static lib only.
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_TEST OFF CACHE BOOL "" FORCE)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/sdl)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/freetype2)
  # add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/cereal)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp)

  # find_package(fmod)
  include_directories("${PROJECT_BINARY_DIR}" glad/include sdl/include freetype2/include glm/include stb_image/ assimp/include)

  set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

  # add property for fmod folder to copy dll files
  set_property(
      TARGET ${PROJECT_NAME}
      APPEND PROPERTY
      FMOD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod"
  )

  # set_target_properties(${PROJECT_NAME}
  # PROPERTIES SUFFIX ".html"
  # LINK_FLAGS " -O3 -sFULL_ES3 -sUSE_SDL=2 -sUSE_FREETYPE=1 -s EXPORTED_RUNTIME_METHODS=['cwrap','setValue','getValue'] --preload-file Assets")
  # target_include_directories(${PROJECT_NAME} INTERFACE src/)
  # target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" include/ src/ vendor/sdl/include vendor/glad/include vendor/freetype2/include vendor/glm vendor/stb_image/ vendor/cereal/include)
  target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" include/ src/ vendor/sdl/include vendor/glad/include vendor/freetype2/include vendor/glm vendor/stb_image/)

  # target_include_directories(${PROJECT_NAME} INTERFACE src/)
  target_link_libraries(${PROJECT_NAME} glad SDL2-static glm freetype assimp)
  # FMOD lives only in the audio module.
  target_link_libraries(${PROJECT_NAME}-audio PUBLIC fmod fmod_studio)

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
          glad freetype glm SDL2-static DESTINATION ${PROJECT_BINARY_DIR}
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

  if(UNIX AND NOT APPLE)
    message(STATUS "LINUX BUILD!")
    add_custom_command(
          TARGET ${PROJECT_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
          ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod/linux/core/lib/x86_64/libfmod.so.14
          ${CMAKE_CURRENT_BINARY_DIR}/libfmod.so.14)

    add_custom_command(
          TARGET ${PROJECT_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
          ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod/linux/studio/lib/x86_64/libfmodstudio.so.14
          ${CMAKE_CURRENT_BINARY_DIR}/libfmodstudio.so.14)
  endif()

  add_subdirectory(example example)
  add_executable(${PROJECT_NAME}ResourcePacker src/timeless/resource_packer.cpp)

  set_target_properties(${PROJECT_NAME} PROPERTIES
      BUILD_RPATH "$ORIGIN"
      INSTALL_RPATH "$ORIGIN"
  )

endif()
