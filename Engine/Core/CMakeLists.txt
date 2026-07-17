cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE GROM_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

file(GLOB_RECURSE GROM_CORE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/Core/*.h"
)

add_library(GromCore STATIC
    ${GROM_CORE_SOURCES}
    ${GROM_CORE_HEADERS}
)

target_include_directories(GromCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

target_compile_features(GromCore PUBLIC cxx_std_20)
