project(fmod)

add_library(fmod SHARED IMPORTED GLOBAL)
add_library(fmod_studio SHARED IMPORTED GLOBAL)

if (WIN32)
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