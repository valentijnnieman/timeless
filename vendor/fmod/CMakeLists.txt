project(fmod)

add_library(fmod SHARED IMPORTED GLOBAL)
add_library(fmod_studio SHARED IMPORTED GLOBAL)

set_property(TARGET fmod PROPERTY
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/apple/core/lib/libfmod.dylib")
set_property(TARGET fmod PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/apple/core/inc")

set_property(TARGET fmod_studio PROPERTY
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/apple/studio/lib/libfmodstudio.dylib")
set_property(TARGET fmod_studio PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/apple/studio/inc")

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