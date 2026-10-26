cmake_minimum_required(VERSION 3.21)

project(
    SemanticCpp
    VERSION 1.0.0
    DESCRIPTION "A future-oriented intelligent stream processing framework for C++"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(SEMANTIC_CPP_BUILD_TESTS "Build the semantic-cpp test suite" ${PROJECT_IS_TOP_LEVEL})

# The library is header-only. Headers are grouped by module and included with
# paths relative to the repository root during development.
set(SEMANTIC_CPP_HEADERS
    bunch/charset.h
    bunch/encode.h
    localisation/localisation.h
    bunch/meta.h
    bunch/point.h
    bunch/character.h
    bunch/unicode_data.h
    bunch/unicode.h
    bunch/sequence.h
    bunch/builder.h
    bunch/buffer.h
    bunch/bunch.h
    structure/function.h
    structure/pool.h
    structure/hash.h
    structure/less.h
    structure/node.h
    structure/tree.h
    structure/ast.h
    structure/trie.h
    structure/segment_tree.h
    structure/character_search_tree.h
    temporal/field.h
    temporal/types.h
    temporal/unit.h
    temporal/overflow.h
    temporal/period.h
    temporal/day.h
    temporal/duration.h
    temporal/era.h
    temporal/month.h
    temporal/week.h
    temporal/year.h
    temporal/time.h
    temporal/calendar.h
    temporal/date_time.h
    temporal/offset.h
    temporal/offset_date_time.h
    temporal/zone.h
    temporal/zone_temporal.h
    temporal/instant.h
    temporal/local_date.h
    temporal/local_date_time.h
    temporal/local_time.h
    temporal/offset_gregory_date_time.h
    temporal/gregory/era.h
    temporal/gregory/year.h
    temporal/gregory/month.h
    temporal/gregory/weekday.h
    temporal/gregory/gregory.h
    temporal/gregory/week_date.h
    temporal/temporal.h
    semantic/collector.h
    semantic/factory.h
    semantic/semantic.h
)

add_library(semantic-cpp INTERFACE)
add_library(semantic-cpp::semantic-cpp ALIAS semantic-cpp)

target_compile_features(semantic-cpp INTERFACE cxx_std_17)
target_include_directories(
    semantic-cpp
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# ---------------------------------------------------------------------------
# Installation
# ---------------------------------------------------------------------------

set(SEMANTIC_CPP_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/semantic-cpp"
    CACHE STRING "Installation directory for semantic-cpp CMake package files")
set(SEMANTIC_CPP_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/semantic-cpp"
    CACHE STRING "Installation directory for semantic-cpp documentation")
mark_as_advanced(SEMANTIC_CPP_INSTALL_CMAKEDIR SEMANTIC_CPP_INSTALL_DOCDIR)

install(
    TARGETS semantic-cpp
    EXPORT semantic-cppTargets
    COMPONENT Development
)

foreach(header IN LISTS SEMANTIC_CPP_HEADERS)
    get_filename_component(headerDirectory "${header}" DIRECTORY)
    install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/${header}"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/semantic-cpp/${headerDirectory}"
        COMPONENT Development
    )
endforeach()

install(
    EXPORT semantic-cppTargets
    FILE semantic-cppTargets.cmake
    NAMESPACE semantic-cpp::
    DESTINATION "${SEMANTIC_CPP_INSTALL_CMAKEDIR}"
    COMPONENT Development
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/semantic-cppConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/semantic-cppConfig.cmake"
    INSTALL_DESTINATION "${SEMANTIC_CPP_INSTALL_CMAKEDIR}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/semantic-cppConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/semantic-cppConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/semantic-cppConfigVersion.cmake"
    DESTINATION "${SEMANTIC_CPP_INSTALL_CMAKEDIR}"
    COMPONENT Development
)

install(
    FILES README.md LICENSE localisation/unicode.md temporal/README.cn.md
    DESTINATION "${SEMANTIC_CPP_INSTALL_DOCDIR}"
    COMPONENT Documentation
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

if(SEMANTIC_CPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
