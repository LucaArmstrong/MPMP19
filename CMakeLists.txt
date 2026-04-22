cmake_minimum_required(VERSION 3.16)
project(MPMP19 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Options
option(MPMP19_NATIVE "Enable -march=native" ON)
option(MPMP19_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# LTO (only applies to Release automatically)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

# Sources
set(SRCS
    src/mpmp19.cpp
    src/Algorithm.cpp
    src/PrimeGapList.cpp
    src/ProgressLogger.cpp
    src/TermBuffer.cpp
    src/Config.cpp
)

# Executable
add_executable(mpmp19
    src/app/main.cpp
    ${SRCS}
)

target_include_directories(mpmp19 PRIVATE src include)

# Dependencies
find_package(OpenMP REQUIRED)
find_package(primesieve REQUIRED)

target_link_libraries(mpmp19
    PRIVATE
        primesieve::primesieve
        OpenMP::OpenMP_CXX
)

# =========================
# Compiler configuration
# =========================
if(MSVC)
    message(FATAL_ERROR "MSVC is not supported (requires __int128 support). Use Clang or GCC.")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Warnings
    target_compile_options(mpmp19 PRIVATE
        -Wall
        -Wextra
        -pedantic
        -Wcast-align
        -Wformat=2
        -Wshadow
        -Wundef
        $<$<BOOL:${MPMP19_WARNINGS_AS_ERRORS}>:-Werror>
    )

    # Debug vs Release
    target_compile_options(mpmp19 PRIVATE
        # Debug: safe + debuggable
        $<$<CONFIG:Debug>:
            -O0
            -g
        >

        # Release: aggressive optimization
        $<$<CONFIG:Release>:
            -O3
            -funroll-loops
            -fomit-frame-pointer
            -fno-semantic-interposition
        >

        # Optional native tuning
        $<$<AND:$<CONFIG:Release>,$<BOOL:${MPMP19_NATIVE}>>:
            -march=native
        >
    )
endif()
