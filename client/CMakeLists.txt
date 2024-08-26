cmake_minimum_required(VERSION 3.20)
project(client)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###################################################################
# Local variables
###################################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(PROJECT_SRC_DIR "src")
set(PROJECT_VENDOR_DIR "vendor")
set(PROJECT_ASSETS_DIR "${CMAKE_CURRENT_LIST_DIR}/assets")

macro(append_linker_flags FLAGS)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS}")
endmacro()
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

target_compile_definitions(${PROJECT_NAME} PRIVATE TB_HEADLESS=0)

target_include_directories(${PROJECT_NAME} PUBLIC ../tabby/src)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SRC_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_ASSETS_DIR})
# target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_VENDOR_DIR}/tabby/vendor/box2c/include)
# target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_VENDOR_DIR}/tabby/vendor/spdlog/include)

add_subdirectory(../tabby ./tabby)


target_link_libraries(${PROJECT_NAME} PRIVATE
        tabby
        tabby_flags
        )

add_custom_target(copy_assets_tabby_example
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_ASSETS_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
        )

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -DDEBUG -DTB_DEBUG=1")
endif()

add_dependencies(${PROJECT_NAME} copy_assets_tabby_example)
