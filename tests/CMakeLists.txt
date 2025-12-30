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
        fixp::fixp
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
        fixp::fixp
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
        fixp::fixp
)

target_compile_features(test_q7_c PRIVATE c_std_23)

add_test(NAME test_q7_c COMMAND test_q7_c)

#-----------------------------------------------------------------------------
# C++ Template Tests
#-----------------------------------------------------------------------------
add_executable(test_cpp_template
    unit/test_cpp_template.cpp
)
target_link_libraries(test_cpp_template PRIVATE fixp::fixp Catch2::Catch2WithMain)
target_compile_features(test_cpp_template PRIVATE cxx_std_23)
add_test(NAME test_cpp_template COMMAND test_cpp_template)

#-----------------------------------------------------------------------------
# Generated Header Tests
#-----------------------------------------------------------------------------
add_executable(test_gen_q8_8
    unit/test_gen_q8_8.c
)
target_link_libraries(test_gen_q8_8 PRIVATE fixp::fixp)
target_compile_features(test_gen_q8_8 PRIVATE c_std_23)
add_test(NAME test_gen_q8_8 COMMAND test_gen_q8_8)

#-----------------------------------------------------------------------------
# Math Functions Tests (C++)
#-----------------------------------------------------------------------------
add_executable(test_math_functions
    unit/test_math_functions.cpp
)
target_link_libraries(test_math_functions PRIVATE fixp::fixp)
target_compile_features(test_math_functions PRIVATE cxx_std_23)
add_test(NAME test_math_functions COMMAND test_math_functions)

#-----------------------------------------------------------------------------
# Math Functions Tests (C23)
#-----------------------------------------------------------------------------
add_library(q15_16_math_impl OBJECT
    ../include/fixp/gen/q15_16_math.c
)
target_include_directories(q15_16_math_impl PUBLIC ../include)
target_compile_features(q15_16_math_impl PRIVATE c_std_23)

add_executable(test_c_math_functions
    unit/test_c_math_functions.c
)
target_link_libraries(test_c_math_functions PRIVATE fixp::fixp q15_16_math_impl m)
target_compile_features(test_c_math_functions PRIVATE c_std_23)
add_test(NAME test_c_math_functions COMMAND test_c_math_functions)

#-----------------------------------------------------------------------------
# DSP Functions Tests (C++)
#-----------------------------------------------------------------------------
add_executable(test_dsp_functions
    unit/test_dsp_functions.cpp
)
target_link_libraries(test_dsp_functions PRIVATE fixp::fixp)
target_compile_features(test_dsp_functions PRIVATE cxx_std_23)
add_test(NAME test_dsp_functions COMMAND test_dsp_functions)
