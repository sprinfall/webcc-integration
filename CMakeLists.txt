cmake_minimum_required(VERSION 3.11)

if(POLICY CMP0074)
    # find_package() uses <PackageName>_ROOT variables.
    # This policy was introduced in CMake version 3.12.
    cmake_policy(SET CMP0074 NEW)
endif()

project(webcc_integration)

set(WEBCC_ENABLE_LOG 1 CACHE STRING "Enable logging? (1:Yes, 0:No)")
set(WEBCC_ENABLE_SSL 0 CACHE STRING "Enable SSL/HTTPS (need OpenSSL)? (1:Yes, 0:No)")
set(WEBCC_ENABLE_GZIP 0 CACHE STRING "Enable gzip compression (need Zlib)? (1:Yes, 0:No)")

set(WEBCC_LOG_LEVEL 2 CACHE STRING "Log level (0:VERB, 1:INFO, 2:USER, 3:WARN or 4:ERRO)")

set(WEBCC_USE_STD_FILESYSTEM 1 CACHE STRING "Use std::filesystem? (1:Yes, 0:No)")
set(WEBCC_USE_STD_STRING_VIEW 1 CACHE STRING "Use std::string_view? (1:Yes, 0:No)")

if(WIN32)
    # Asio needs this!
    # 0x0601 means Win7. So our application targets Win7 and above.
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

# C++ standard requirements.

if(WEBCC_USE_STD_FILESYSTEM OR WEBCC_USE_STD_STRING_VIEW)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Boost

set(BOOST_COMPONENTS system date_time)

if(NOT WEBCC_USE_STD_FILESYSTEM)
    set(BOOST_COMPONENTS ${BOOST_COMPONENTS} filesystem)
endif()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.66.0 REQUIRED COMPONENTS ${BOOST_COMPONENTS})
if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIRS})
endif()

if(WEBCC_ENABLE_SSL)
    # NOTE: Don't use static libs!
    #   set(OPENSSL_USE_STATIC_LIBS ON)
    #   set(OPENSSL_MSVC_STATIC_RT ON)
    find_package(OpenSSL)
    if(OPENSSL_FOUND)
        include_directories(${OPENSSL_INCLUDE_DIR})
        message(STATUS "OpenSSL libs: " ${OPENSSL_LIBRARIES})
    endif()
endif()

if(WEBCC_ENABLE_GZIP)
    find_package(ZLIB REQUIRED)
    if(ZLIB_FOUND)
        # You can link to ${ZLIB_LIBRARIES} or the imported target ZLIB::ZLIB.
        include_directories(${ZLIB_INCLUDE_DIRS})
    endif()
endif()

# For including webcc headers as "webcc/client.h".
include_directories(third_party)

# For including config.h as "webcc/config.h".
include_directories(${PROJECT_BINARY_DIR}/third_party)

add_subdirectory(third_party/webcc)

add_subdirectory(src)
