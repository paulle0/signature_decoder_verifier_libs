# FindSodium.cmake -- locate libsodium and expose it as Sodium::Sodium.
#
# Mirrors what the Makefile did by hand: ask pkg-config first, and fall back
# to a plain header/library search when pkg-config is unavailable or knows
# nothing about libsodium (common on macOS/Homebrew without PKG_CONFIG_PATH).
#
# Result variables:
#   Sodium_FOUND        libsodium was located
#   Sodium_INCLUDE_DIRS directory holding sodium.h
#   Sodium_LIBRARIES    the library to link
#   Sodium_VERSION      version, when pkg-config could report one
#
# Imported target:
#   Sodium::Sodium

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PC_Sodium QUIET libsodium)
endif()

find_path(Sodium_INCLUDE_DIR
    NAMES sodium.h
    HINTS ${PC_Sodium_INCLUDE_DIRS}
    PATH_SUFFIXES sodium
)

find_library(Sodium_LIBRARY
    NAMES sodium libsodium
    HINTS ${PC_Sodium_LIBRARY_DIRS}
)

if(PC_Sodium_VERSION)
    set(Sodium_VERSION "${PC_Sodium_VERSION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sodium
    REQUIRED_VARS Sodium_LIBRARY Sodium_INCLUDE_DIR
    VERSION_VAR   Sodium_VERSION
    FAIL_MESSAGE  "libsodium development files not found. Install libsodium-dev (Debian/Ubuntu), libsodium-devel (Fedora/RHEL) or `brew install libsodium` (macOS), then re-run cmake."
)

if(Sodium_FOUND)
    set(Sodium_INCLUDE_DIRS "${Sodium_INCLUDE_DIR}")
    set(Sodium_LIBRARIES    "${Sodium_LIBRARY}")

    if(NOT TARGET Sodium::Sodium)
        add_library(Sodium::Sodium UNKNOWN IMPORTED)
        set_target_properties(Sodium::Sodium PROPERTIES
            IMPORTED_LOCATION             "${Sodium_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Sodium_INCLUDE_DIR}")
    endif()
endif()

mark_as_advanced(Sodium_INCLUDE_DIR Sodium_LIBRARY)
