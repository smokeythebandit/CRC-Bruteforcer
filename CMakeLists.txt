cmake_minimum_required(VERSION 3.0)
project(CRC-Bruteforcer)

# Fetch argparse
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG        v2.9
)
FetchContent_MakeAvailable(argparse)

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

# Optimization
if(MSVC)
    set(CMAKE_CXX_FLAGS "/O2")
else()
    set(CMAKE_CXX_FLAGS "-O3")
endif()
add_executable(${CMAKE_PROJECT_NAME} main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} argparse CRCpp spdlog)
