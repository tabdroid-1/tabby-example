cmake_minimum_required(VERSION 3.20)
project(game
	VERSION 0.0.1
    DESCRIPTION "Tabby engine example"
	HOMEPAGE_URL "https://github.com/tabdroid-1/tabby-example"
    LANGUAGES CXX
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
    message(STATUS "Including Objective-C++ for APPLE")
    enable_language(OBJC)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 14)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
    set(CMAKE_OBJCXX_EXTENSIONS OFF)
endif()

###################################################################
# Local variables
###################################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(PROJECT_SRC_DIR "src")
set(PROJECT_VENDOR_DIR "vendor")
set(PROJECT_ASSETS_DIR "${CMAKE_CURRENT_LIST_DIR}/assets")

###################################################################
# Target sources and create executable
###################################################################
file(GLOB_RECURSE PROJECT_SOURCES
        "${PROJECT_SRC_DIR}/*.h"
        "${PROJECT_SRC_DIR}/*.cpp"
        )

list(APPEND PROJECT_SOURCES
        )

if (ANDROID)
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})
else()
    add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
endif()

set(TB_HEADLESS OFF CACHE BOOL "Enable headless mode" FORCE)
set(TB_NETWORKING OFF CACHE BOOL "Enable networking" FORCE)

target_include_directories(${PROJECT_NAME} PUBLIC vendor/tabby/src)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SRC_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_ASSETS_DIR})


if (NOT TARGET tabby)
add_subdirectory(vendor/tabby)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
        tabby
        tabby_flags
        )

add_custom_target(copy_assets_client
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_ASSETS_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
        )

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -DDEBUG -DTB_DEBUG=1")
endif()

add_dependencies(${PROJECT_NAME} copy_assets_client)


tabby_compile_shader(
    TYPE VERTEX
    SHADERS assets/shaders/mesh_shader/vs.sc
    VARYING_DEF assets/shaders/mesh_shader/varying.def.sc
    OUTPUT_DIR assets/shaders/mesh_shader
    OUT_FILES_VAR GENERATED_SHADER_OUTPUTS
    INCLUDE_DIRS assets/shaders/common.sh
    AS_HEADERS FALSE
)

