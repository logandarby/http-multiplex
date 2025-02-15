cmake_minimum_required(VERSION 3.25)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(DZHttp)

# Sources
# MY_SOURCES is defined to be a list of all the source files in src that aren't main.cpp
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM MY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
message("${MY_SOURCES}")
# file(GLOB_RECURSE MY_INCLUDE CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

add_executable("${CMAKE_PROJECT_NAME}")
set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY C_STANDARD 99)

# Configure resource files macro
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/res/"
        _DEBUG
    )
endif()

# Add Tests

enable_testing()
add_subdirectory(tests)

# Final Link
add_library("${CMAKE_PROJECT_NAME}_Lib" STATIC ${MY_SOURCES})

# External Libraries
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
target_link_libraries("${CMAKE_PROJECT_NAME}" PUBLIC m "${CMAKE_PROJECT_NAME}_Lib")
target_include_directories("${CMAKE_PROJECT_NAME}_Lib" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

