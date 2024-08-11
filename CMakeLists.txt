cmake_minimum_required(VERSION 3.20)
project(tabby-example)
set(CMAKE_CXX_STANDARD 17)
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
        ${PROJECT_VENDOR_DIR}/tinyobjloader/tinyobjloader.cpp
        )


if (ANDROID)
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})
else()
    add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SRC_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_ASSETS_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_VENDOR_DIR}/tabby/src)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_VENDOR_DIR}/tinyobjloader)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_VENDOR_DIR}/tabby/vendor/box2c/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_VENDOR_DIR}/tabby/vendor/spdlog/include)

add_subdirectory(${PROJECT_VENDOR_DIR}/tabby)


target_link_libraries(${PROJECT_NAME}
        tabby
        )

if (EMSCRIPTEN)
	append_linker_flags("-sOFFSCREEN_FRAMEBUFFER=1")
    append_linker_flags("-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2")
    append_linker_flags("-sUSE_SDL=2")
    append_linker_flags("-FULL_ES3=1")
    append_linker_flags("-lGL")
    append_linker_flags("-lopenal")
    append_linker_flags("-sFORCE_FILESYSTEM=1")
    append_linker_flags("-sFETCH=1")
    append_linker_flags("-sASYNCIFY")
    append_linker_flags("-sASSERTIONS=1")
    append_linker_flags("--preload-file assets")
	append_linker_flags("--use-port=sdl2")
	append_linker_flags("-sPROXY_TO_PTHREAD")
	append_linker_flags("-sALLOW_MEMORY_GROWTH")
	append_linker_flags("-sPTHREAD_POOL_SIZE=1")
	target_link_options(${PROJECT_NAME} PUBLIC -sNO_DISABLE_EXCEPTION_CATCHING)
    target_link_options(${PROJECT_NAME} PUBLIC -pthread)
    target_compile_options(${PROJECT_NAME} PUBLIC -pthread)
 
	append_linker_flags("-O3")
	append_linker_flags("-sMIN_FIREFOX_VERSION=79")
	append_linker_flags("-sMIN_SAFARI_VERSION=150000")
	append_linker_flags("-sMIN_IE_VERSION=0x7FFFFFFF") # Do not support Internet Explorer at all (this is the Emscripten default, shown here for posterity)
	# append_linker_flags("-sMIN_EDGE_VERSION=79")
	append_linker_flags("-sMIN_CHROME_VERSION=80")

    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="/assets/")
endif()

add_custom_target(copy_assets_tabby_example
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_ASSETS_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
        )

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -DDEBUG -DTB_DEBUG=1")
endif()

add_dependencies(${PROJECT_NAME} copy_assets_tabby_example)
