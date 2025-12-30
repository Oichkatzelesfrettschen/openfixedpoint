cmake_minimum_required(VERSION 3.25)

# Enable testing for this directory
enable_testing()

#-----------------------------------------------------------------------------
# Fetch Catch2 for C++ Testing
#-----------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)

#-----------------------------------------------------------------------------
# C++ Tests
#-----------------------------------------------------------------------------
add_executable(test_q16_16_cpp
    unit/test_q16_16.cpp
)

target_link_libraries(test_q16_16_cpp
    PRIVATE
        libfixp::libfixp
        Catch2::Catch2WithMain
)

# C++23 features
target_compile_features(test_q16_16_cpp PRIVATE cxx_std_23)

# Register test
add_test(NAME test_q16_16_cpp COMMAND test_q16_16_cpp)

#-----------------------------------------------------------------------------
# C Tests (using a simple custom runner or Unity/CMocka if needed)
# For now, a simple main() based test for C23 compliance
#-----------------------------------------------------------------------------
add_executable(test_q16_16_c
    unit/test_q16_16.c
)

target_link_libraries(test_q16_16_c
    PRIVATE
        libfixp::libfixp
)

# C23 features
target_compile_features(test_q16_16_c PRIVATE c_std_23)

add_test(NAME test_q16_16_c COMMAND test_q16_16_c)

#-----------------------------------------------------------------------------
# Q7 Tests
#-----------------------------------------------------------------------------
add_executable(test_q7_c
    unit/test_q7.c
)

target_link_libraries(test_q7_c
    PRIVATE
        libfixp::libfixp
)

target_compile_features(test_q7_c PRIVATE c_std_23)

add_test(NAME test_q7_c COMMAND test_q7_c)

#-----------------------------------------------------------------------------
# C++ Template Tests
#-----------------------------------------------------------------------------
add_executable(test_cpp_template
    unit/test_cpp_template.cpp
)
target_link_libraries(test_cpp_template PRIVATE libfixp::libfixp Catch2::Catch2WithMain)
target_compile_features(test_cpp_template PRIVATE cxx_std_23)
add_test(NAME test_cpp_template COMMAND test_cpp_template)

#-----------------------------------------------------------------------------
# Generated Header Tests
#-----------------------------------------------------------------------------
add_executable(test_gen_q8_8
    unit/test_gen_q8_8.c
)
target_link_libraries(test_gen_q8_8 PRIVATE libfixp::libfixp)
target_compile_features(test_gen_q8_8 PRIVATE c_std_23)
add_test(NAME test_gen_q8_8 COMMAND test_gen_q8_8)
