cmake_minimum_required(VERSION 3.5)

project(LibSOFA C)

file(GLOB sofa_sources *.c)
list(REMOVE_ITEM sofa_sources ${CMAKE_CURRENT_SOURCE_DIR}/t_sofa_c.c)

add_library(LibSOFA STATIC ${sofa_sources} sofa.h sofam.h)

target_include_directories(LibSOFA PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(LibSOFA PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
