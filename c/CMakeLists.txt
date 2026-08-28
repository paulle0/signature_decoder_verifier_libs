# Builds libsdv.a (the file hasher) and its tests.
#
#   cmake -S . -B build              configure
#   cmake --build build              build the static library
#   ctest --test-dir build           run the tests
#   rm -rf build                     "make clean"
#
# SHA-256 and BLAKE2b both come from libsodium, linked dynamically. Install the
# development package before configuring:
#
#   Debian/Ubuntu   apt-get install libsodium-dev
#   Fedora/RHEL     dnf install libsodium-devel
#   macOS           brew install libsodium

cmake_minimum_required(VERSION 3.15)

project(sdv
        VERSION 0.1.0
        DESCRIPTION "Signature decoder/verifier support library"
        LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Tests are for developers of this library; a consumer that pulls us in via
# add_subdirectory()/FetchContent gets the library only.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(SDV_IS_TOP_LEVEL ON)
else()
    set(SDV_IS_TOP_LEVEL OFF)
endif()

option(SDV_BUILD_TESTS "Build the sdv test suite" ${SDV_IS_TOP_LEVEL})
option(SDV_INSTALL     "Generate install rules for sdv" ${SDV_IS_TOP_LEVEL})

# Default to an optimised build so a bare `cmake -S . -B build` matches the
# Makefile's -O2 instead of producing an unoptimised, flagless build.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Fails with an actionable message instead of a confusing "sodium.h: No such
# file or directory" halfway through the build. See cmake/FindSodium.cmake.
find_package(Sodium REQUIRED)

# --------------------------------------------------------------------------
# The library
# --------------------------------------------------------------------------

add_library(sdv STATIC
    src/filehasher.c
)
add_library(sdv::sdv ALIAS sdv)

# Archive name: libsdv.a, as with the Makefile.
set_target_properties(sdv PROPERTIES
    OUTPUT_NAME       sdv
    C_STANDARD        99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS      OFF
    POSITION_INDEPENDENT_CODE ON   # so consumers may link us into a .so
)

target_include_directories(sdv
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# PUBLIC, not PRIVATE: a static archive cannot record its own dependency on
# libsodium, so the Makefile made every consumer add -lsodium by hand. Here
# the requirement travels with the target -- anything linking sdv::sdv gets
# libsodium's include dirs and -lsodium automatically.
target_link_libraries(sdv PUBLIC Sodium::Sodium)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(sdv PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(sdv PRIVATE /W4)
endif()

# --------------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------------

if(SDV_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# --------------------------------------------------------------------------
# Install / packaging
#
# After `cmake --install build --prefix /some/where`, a consumer can do:
#
#   find_package(sdv REQUIRED)
#   target_link_libraries(myapp PRIVATE sdv::sdv)
#
# and needs to say nothing about libsodium.
# --------------------------------------------------------------------------

if(SDV_INSTALL)
    install(TARGETS sdv
            EXPORT sdvTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

    install(DIRECTORY include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(EXPORT sdvTargets
            FILE      sdvTargets.cmake
            NAMESPACE sdv::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdv)

    # sdvConfig.cmake re-runs find_package(Sodium), so the find module has to
    # travel with it.
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindSodium.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdv)

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdvConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/sdvConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdv)

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/sdvConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion)

    install(FILES
                "${CMAKE_CURRENT_BINARY_DIR}/sdvConfig.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/sdvConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sdv)
endif()
