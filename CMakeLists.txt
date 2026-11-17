cmake_minimum_required(VERSION 3.19)

project(timeless VERSION 0.4.1)

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
if(APPLE)
  # assimp's bundled zlib defines fdopen() away on macOS, which breaks against
  # current SDKs; macOS ships zlib, so use that instead.
  set(ASSIMP_BUILD_ZLIB OFF)
else()
  set(ASSIMP_BUILD_ZLIB ON)
endif()

option(EMSCRIPTEN "WASM build or regular" OFF)
option(EMSCRIPTEN_PTHREADS "PThread support" OFF)
# Threaded WASM builds only: mix audio in an AudioWorklet (see below).
option(TIMELESS_AUDIO_WORKLETS "WASM: mix audio in an AudioWorklet (threaded builds)" ON)

# ---------------------------------------------------------------------------
# Modular sub-targets
#
#   timeless        — core engine (components, rendering, input, window, ...)
#   timeless-audio  — SoundSystem, on the tokkel audio library (the ONLY target
#                     that links tokkel, and through it miniaudio and STK)
#   timeless-ai     — NpcAiSystem time-sequencing (rewind/replay)
#
# The optional feature modules depend on core one-way; core never references
# them (verified: SoundSystem/NpcAiSystem are each referenced only by their own
# translation unit). So a game links just the modules it uses — e.g. BiasBusters
# links `timeless timeless-audio` and pays nothing for the AI module or, if it
# dropped audio too, for tokkel.
# ---------------------------------------------------------------------------

# Feature-module sources, carved out of the source tree.
set(TIMELESS_AUDIO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/timeless/systems/sound_system.cpp"
                           "${CMAKE_CURRENT_SOURCE_DIR}/src/timeless/music.cpp")
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
# nlohmann/json, for the scores music.cpp reads; private to the audio module.
target_include_directories(${PROJECT_NAME}-audio PRIVATE vendor/nlohmann)
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
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/tokkel)
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
    # Mix the audio in an AudioWorklet, on the browser's audio thread. Without
    # it miniaudio uses a ScriptProcessorNode, whose callback runs on the main
    # thread, so every long frame (a scene loading) starves the audio and it
    # crackles. Worklets need shared memory, hence threaded builds only.
    # -sASYNCIFY because miniaudio's device init waits for the worklet with
    # emscripten_sleep() (see miniaudio.h, "AudioWorklets").
    if(TIMELESS_AUDIO_WORKLETS)
      target_compile_definitions(tokkel_miniaudio PRIVATE MA_ENABLE_AUDIO_WORKLETS)
      target_link_options(${PROJECT_NAME} PUBLIC
        -sAUDIO_WORKLET=1
        -sWASM_WORKERS=1
        -sASYNCIFY
      )
    endif()
    target_link_options(${PROJECT_NAME} PUBLIC
      -sALLOW_MEMORY_GROWTH=1
      # Up to the 4GB a 32-bit wasm can address, rather than the 2GB default:
      # headroom for the biggest cases' crowds.
      -sMAXIMUM_MEMORY=4GB
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
      # Up to the 4GB a 32-bit wasm can address, rather than the 2GB default:
      # headroom for the biggest cases' crowds.
      -sMAXIMUM_MEMORY=4GB
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
  # tokkel lives only in the audio module. PRIVATE: no engine header includes
  # it, and CMake still carries a static library's dependencies to the final link.
  target_link_libraries(${PROJECT_NAME}-audio PRIVATE tokkel::tokkel)

  # add_subdirectory(example example)

else()
  # SDL2 (vendored) replaces GLFW for windowing/input. Build the static lib only.
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_TEST OFF CACHE BOOL "" FORCE)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/sdl)
  if(NOT IOS) # iOS uses the system's OpenGL ES, no loader
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad)
  endif()
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/freetype2)
  # add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/cereal)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/tokkel)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp)

  include_directories("${PROJECT_BINARY_DIR}" glad/include sdl/include freetype2/include glm/include stb_image/ assimp/include)

  set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

  # set_target_properties(${PROJECT_NAME}
  # PROPERTIES SUFFIX ".html"
  # LINK_FLAGS " -O3 -sFULL_ES3 -sUSE_SDL=2 -sUSE_FREETYPE=1 -s EXPORTED_RUNTIME_METHODS=['cwrap','setValue','getValue'] --preload-file Assets")
  # target_include_directories(${PROJECT_NAME} INTERFACE src/)
  # target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" include/ src/ vendor/sdl/include vendor/glad/include vendor/freetype2/include vendor/glm vendor/stb_image/ vendor/cereal/include)
  target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" include/ src/ vendor/sdl/include vendor/glad/include vendor/freetype2/include vendor/glm vendor/stb_image/)

  # target_include_directories(${PROJECT_NAME} INTERFACE src/)
  if(IOS)
    # OpenGL ES comes from the system. SDL2main supplies the app's real main(),
    # which starts UIKit and calls the game's main() (renamed SDL_main by
    # <SDL_main.h>).
    target_link_libraries(${PROJECT_NAME} SDL2-static SDL2main glm freetype assimp "-framework OpenGLES")
  else()
    target_link_libraries(${PROJECT_NAME} glad SDL2-static glm freetype assimp)
  endif()
  # tokkel lives only in the audio module. PRIVATE: no engine header includes
  # it, and CMake still carries a static library's dependencies to the final link.
  target_link_libraries(${PROJECT_NAME}-audio PRIVATE tokkel::tokkel)

  # Desktop tools: an iOS build packs its assets with a host-built packer (see
  # the game's CMakeLists).
  if(NOT IOS)
    add_subdirectory(example example)
    add_executable(${PROJECT_NAME}ResourcePacker src/timeless/resource_packer.cpp)
  endif()

  set_target_properties(${PROJECT_NAME} PROPERTIES
      BUILD_RPATH "$ORIGIN"
      INSTALL_RPATH "$ORIGIN"
  )

endif()

# ---------------------------------------------------------------------------
# Header self-containment check (off by default).
#
# Every public header must compile as the first and only include of a
# translation unit, so that include order never matters. With
#   -DTIMELESS_CHECK_HEADERS=ON
# one tiny source per header is generated and compiled as `timeless-header-check`;
# a header that leans on something included before it fails to build there.
# ---------------------------------------------------------------------------
option(TIMELESS_CHECK_HEADERS "Compile every public header on its own" OFF)
if(TIMELESS_CHECK_HEADERS)
  file(GLOB_RECURSE TIMELESS_PUBLIC_HEADERS
      RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include"
      "${CMAKE_CURRENT_SOURCE_DIR}/include/timeless/*.hpp")
  # Unused, and needs cereal, which isn't on the include path.
  list(FILTER TIMELESS_PUBLIC_HEADERS EXCLUDE REGEX "algorithm/ud_graph\\.hpp$")

  set(TIMELESS_HEADER_CHECK_SOURCES "")
  foreach(header ${TIMELESS_PUBLIC_HEADERS})
    string(REPLACE "/" "_" check_name "${header}")
    set(check_source "${CMAKE_CURRENT_BINARY_DIR}/header_check/${check_name}.cpp")
    file(CONFIGURE OUTPUT "${check_source}"
         CONTENT "#include \"${header}\"\n" @ONLY)
    list(APPEND TIMELESS_HEADER_CHECK_SOURCES "${check_source}")
  endforeach()

  add_library(${PROJECT_NAME}-header-check OBJECT ${TIMELESS_HEADER_CHECK_SOURCES})
  target_link_libraries(${PROJECT_NAME}-header-check PRIVATE
      ${PROJECT_NAME} ${PROJECT_NAME}-audio ${PROJECT_NAME}-ai)
endif()
