cmake_minimum_required(VERSION 3.31)
project(GPlayer)

set(CMAKE_CXX_STANDARD 23)

# Set FMOD path
set(FMOD_ROOT "/opt/fmod")  # Adjust this path
set(FMOD_CORE_PATH "${FMOD_ROOT}/api/core")

# Find FMOD headers
find_path(FMOD_INCLUDE_DIR
        NAMES fmod.hpp
        PATHS ${FMOD_CORE_PATH}/inc
)

# Find FMOD libraries
find_library(FMOD_LIBRARY
        NAMES fmod
        PATHS ${FMOD_CORE_PATH}/lib/x86_64  # or x86 for 32-bit
)

find_package(OpenSSL REQUIRED)

add_executable(GPlayer main.cpp
        SimpleHTTPServer.cpp
        SimpleHTTPServer.h
        Player.cpp
        Player.h)

# Link FMOD
target_include_directories(GPlayer PRIVATE ${FMOD_INCLUDE_DIR})
target_link_libraries(GPlayer ${FMOD_LIBRARY} OpenSSL::SSL OpenSSL::Crypto)
target_compile_definitions(${PROJECT_NAME} PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)