cmake_minimum_required(VERSION 3.16)

project(CampusRPG LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
    target_compile_options(CampusRPG PRIVATE /W4 /utf-8)
else()
    target_compile_options(CampusRPG PRIVATE -Wall -Wextra -Wpedantic)
endif()
