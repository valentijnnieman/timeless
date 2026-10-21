project(fmod)

# On macOS the dylibs are SHARED imports, so CMake puts their folder on the
# binaries' build RPATH and the game finds them at launch (their install name
# is @rpath/libfmod*.dylib). Elsewhere (iOS included) they are STATIC imports.
if(APPLE AND NOT IOS)
  add_library(fmod SHARED IMPORTED GLOBAL)
  add_library(fmod_studio SHARED IMPORTED GLOBAL)
else()
  add_library(fmod STATIC IMPORTED GLOBAL)
  add_library(fmod_studio STATIC IMPORTED GLOBAL)
endif()

if(IOS)
# FMOD ships one static library per SDK: pick the device or the simulator one
# by the SDK this build targets (CMAKE_OSX_SYSROOT=iphoneos|iphonesimulator).
if(CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator")
  set(FMOD_IOS_SDK iphonesimulator)
else()
  set(FMOD_IOS_SDK iphoneos)
endif()
set_property(TARGET fmod PROPERTY
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/ios/core/lib/libfmod_${FMOD_IOS_SDK}.a")
set_property(TARGET fmod PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/ios/core/inc")
set_property(TARGET fmod PROPERTY
    INTERFACE_LINK_LIBRARIES "-framework AudioToolbox" "-framework AVFoundation")

set_property(TARGET fmod_studio PROPERTY
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/ios/studio/lib/libfmodstudio_${FMOD_IOS_SDK}.a")
set_property(TARGET fmod_studio PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/ios/studio/inc")
set_property(TARGET fmod_studio PROPERTY
    INTERFACE_LINK_LIBRARIES fmod)
elseif(APPLE)
set_property(TARGET fmod PROPERTY
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/apple/core/lib/libfmod.dylib")
set_property(TARGET fmod PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/apple/core/inc")

set_property(TARGET fmod_studio PROPERTY
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/apple/studio/lib/libfmodstudio.dylib")
set_property(TARGET fmod_studio PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/apple/studio/inc")
endif()
if(EMSCRIPTEN)
  if(EMSCRIPTEN_PTHREADS)
    set_property(TARGET fmod PROPERTY
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/html5/core/lib/w32/fmodP_wasm.a")
    set_property(TARGET fmod PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/html5/core/inc")

    set_property(TARGET fmod_studio PROPERTY
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/html5/studio/lib/w32/fmodstudioP_wasm.a")
    set_property(TARGET fmod_studio PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/html5/studio/inc")
  else()
    set_property(TARGET fmod PROPERTY
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/html5/core/lib/w32/fmod_wasm.a")
    set_property(TARGET fmod PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/html5/core/inc")

    set_property(TARGET fmod_studio PROPERTY
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/html5/studio/lib/w32/fmodstudio_wasm.a")
    set_property(TARGET fmod_studio PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/html5/studio/inc")
  endif()
endif()
if(LINUX)
set_property(TARGET fmod PROPERTY
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/linux/core/lib/x86_64/libfmod.so")
set_property(TARGET fmod PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/linux/core/inc")

set_property(TARGET fmod_studio PROPERTY
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/linux/studio/lib/x86_64/libfmodstudio.so")
set_property(TARGET fmod_studio PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/linux/studio/inc")
endif()

if(WIN32)
    set_property(TARGET fmod PROPERTY
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/win/core/lib/fmod.dll")
    set_property(TARGET fmod PROPERTY
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/win/core/lib/fmod_vc.lib")
    set_property(TARGET fmod PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/win/core/inc")

    set_property(TARGET fmod_studio PROPERTY
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/win/studio/lib/fmodstudio.dll")
    set_property(TARGET fmod_studio PROPERTY
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/win/studio/lib/fmodstudio_vc.lib")
    set_property(TARGET fmod_studio PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/win/studio/inc")
endif()
