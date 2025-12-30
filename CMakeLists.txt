cmake_minimum_required(VERSION 3.25)

project(libfixp
    VERSION 0.1.0
    DESCRIPTION "Universal Fixed-Point Mathematics Library"
    LANGUAGES C CXX
)

#-----------------------------------------------------------------------------
# Options
#-----------------------------------------------------------------------------
option(BUILD_TESTING "Build the testing tree" ON)
option(ENABLE_LINTING "Enable linting (clang-tidy)" ON)

#-----------------------------------------------------------------------------
# Standard Compliance
#-----------------------------------------------------------------------------
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#-----------------------------------------------------------------------------
# Library Definition
#-----------------------------------------------------------------------------
# Header-only library
add_library(fixp INTERFACE)
add_library(fixp::fixp ALIAS fixp)

target_include_directories(fixp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fixp INTERFACE
    c_std_23
    cxx_std_23
)

#-----------------------------------------------------------------------------
# Compiler Warnings (Treat Warnings as Errors)
#-----------------------------------------------------------------------------
if(MSVC)
    target_compile_options(fixp INTERFACE
        /W4 /WX /permissive-
    )
else()
    target_compile_options(fixp INTERFACE
        -Wall -Wextra -Wpedantic -Werror
        -Wconversion -Wsign-conversion -Wshadow
    )
endif()

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if(BUILD_TESTING)
    include(CTest)
    add_subdirectory(tests)
endif()

#-----------------------------------------------------------------------------
# Installation
#-----------------------------------------------------------------------------
include(GNUInstallDirs)

install(TARGETS fixp
    EXPORT fixpTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT fixpTargets
    FILE fixpTargets.cmake
    NAMESPACE fixp::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fixp
)
