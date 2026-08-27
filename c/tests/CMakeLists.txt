# Links against the archive exactly as a consumer would -- except that
# sdv::sdv carries its libsodium dependency along, so unlike the Makefile
# there is no -lsodium to name here.

add_executable(test_filehasher sdv/test_filehasher.c)

target_link_libraries(test_filehasher PRIVATE sdv::sdv)

set_target_properties(test_filehasher PROPERTIES
    C_STANDARD          99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS        OFF
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(test_filehasher PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(test_filehasher PRIVATE /W4)
endif()

# The fixture is passed by absolute path so the test does not care about the
# working directory ctest happens to use.
add_test(NAME filehasher
         COMMAND test_filehasher "${CMAKE_CURRENT_SOURCE_DIR}/data/test.txt")
