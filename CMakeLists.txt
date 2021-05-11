cmake_minimum_required(VERSION 3.13)
project(GenY VERSION "0.0.0")

# # set compiler path
#set(CMAKE_C_COMPILER "/usr/bin/cc")
#set(CMAKE_CXX_COMPILER "/usr/bin/c++")
#set(CMAKE_CXX_COMPILER_ID "Apple Clang")

# C++17 standard support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# enable all warnings and treat them as errors
if(MSVC)
  add_compile_options(/W3 /WX)
elseif(APPLE)
  MESSAGE(STATUS "This is apple")
  add_compile_options(-Wall -Og -ll -v -g)
else()
  MESSAGE(STATUS "This is Linux")
  add_compile_options(-Wall -O2 -lfl)
endif()

# some definitions
add_compile_definitions(APP_NAME="GenY Compiler")
add_compile_definitions(APP_VERSION="${PROJECT_VERSION}")
add_compile_definitions(APP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
add_compile_definitions(APP_VERSION_MINOR=${PROJECT_VERSION_MINOR})
add_compile_definitions(APP_VERSION_PATCH=${PROJECT_VERSION_PATCH})

# project include directories
include_directories(src)
include_directories(3rdparty/xstl)

# all of C++ source files
file(GLOB_RECURSE SOURCES_C "src/*.c")
file(GLOB_RECURSE SOURCES_CXX "src/*.cpp" "src/*.cc")

set_source_files_properties(${SOURCES_C} PROPERTIES LANGUAGE CXX )

# executable
add_executable(frontend ${SOURCES_C} ${SOURCES_CXX})
