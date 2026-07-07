cmake_minimum_required(VERSION 3.16)

project(CampusRPG LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_CURL_AI "Enable AI assistant HTTP requests through libcurl" ON)

file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(CampusRPG
    main.cpp
    ${PROJECT_SOURCES}
)

target_include_directories(CampusRPG PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(MSVC)
    target_compile_options(CampusRPG PRIVATE /W4)
else()
    target_compile_options(CampusRPG PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(ENABLE_CURL_AI)
    find_package(CURL QUIET)
    if(CURL_FOUND)
        target_compile_definitions(CampusRPG PRIVATE ENABLE_CURL_AI)
        target_link_libraries(CampusRPG PRIVATE CURL::libcurl)
        message(STATUS "libcurl found: AI assistant network mode enabled.")
    else()
        message(WARNING "libcurl not found: building without AI network mode.")
    endif()
endif()
