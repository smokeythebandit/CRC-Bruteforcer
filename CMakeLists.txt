cmake_minimum_required(VERSION 3.0)
project(CRC-Bruteforcer)

# Fetch Depdenencies
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG        v2.9
)
FetchContent_MakeAvailable(argparse)

include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        9.1.0
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    CRCpp
    GIT_REPOSITORY https://github.com/d-bahr/CRCpp.git
    GIT_TAG        release-1.2.0.0
)
FetchContent_MakeAvailable(CRCpp)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.11.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    indicators
    GIT_REPOSITORY https://github.com/p-ranav/indicators.git
    GIT_TAG        v2.2
)
FetchContent_MakeAvailable(indicators)

# Optimization
if(MSVC)
    set(CMAKE_CXX_FLAGS "/O2 /EHs")
else()
    set(CMAKE_CXX_FLAGS "-O3")
endif()
add_compile_definitions(CRCPP_USE_CPP11)

set (CMAKE_CXX_STANDARD 17)
add_executable(${CMAKE_PROJECT_NAME} main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} argparse CRCpp spdlog indicators fmt)
