cmake_minimum_required(VERSION 3.20)
project(tg2027_ide LANGUAGES C)

file(GLOB_RECURSE TG2027_C_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c"
)
file(GLOB_RECURSE TG2027_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

if(DEFINED ENV{DEVKITPRO})
    set(DEVKITPRO_ROOT "$ENV{DEVKITPRO}")
else()
    set(DEVKITPRO_ROOT "/opt/devkitpro")
endif()

set_property(SOURCE ${TG2027_C_SOURCES} PROPERTY INCLUDE_DIRECTORIES
    "${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_CURRENT_SOURCE_DIR}/build;${DEVKITPRO_ROOT}/libgba/include"
)

# This target exists only for CLion source navigation; build the ROM with ./build.sh.
add_custom_target(tg2027_ide_sources SOURCES
    ${TG2027_C_SOURCES}
    ${TG2027_HEADERS}
)
