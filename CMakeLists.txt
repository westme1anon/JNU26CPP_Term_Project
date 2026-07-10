cmake_minimum_required(VERSION 3.16)

project(CampusRPG LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 2. 引入 FetchContent 模块，用于自动下载 SFML
include(FetchContent)

message(STATUS "正在准备引入 SFML 3.0...")
# === 核心：直接读取本地解压的 SFML 源码 ===
set(SFML_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SFML")
add_subdirectory(${SFML_SOURCE_DIR} sfml_build)

file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(CampusRPG
    main.cpp
    ${PROJECT_SOURCES}
)

# 4. 将 SFML 的模块链接到你们的游戏项目中
# SFML::Graphics 包含了图形、窗口、系统等核心功能；如果需要声音，可以加上 SFML::Audio
target_link_libraries(CampusRPG PRIVATE 
    SFML::Graphics 
    SFML::Audio
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



# 5. 【重要】设置资源拷贝（图片、音效等）
# 确保你们在写代码加载素材（如 "assets/hero.png"）时，程序在编译输出目录能找到 assets 文件夹
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/assets")
    file(CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${CMAKE_CURRENT_BINARY_DIR}/assets" SYMBOLIC)
endif()