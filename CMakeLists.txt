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
  add_compile_options(-Wall -O2 -ll)
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
#
#find_package(BISON REQUIRED)
#find_package(FLEX REQUIRED)
#ADD_CUSTOM_COMMAND(
#        SOURCE ${CMAKE_SOURCE_DIR}/src/frontend/sysy.y
#        COMMAND ${BISON_EXECUTABLE}
#        ARGS -d
#        OUTPUT ${CMAKE_SOURCE_DIR}/src/frontend/sysy.tab.c
#        COMMENT "Generating sysy.tab.c sysy.tab.h"
#)
#ADD_CUSTOM_COMMAND(
#        SOURCE ${CMAKE_SOURCE_DIR}/src/frontend/sysy.l
#        COMMAND ${FLEX_EXECUTABLE}
#        OUTPUT ${CMAKE_SOURCE_DIR}/src/frontend/lex.yy.c
#        COMMENT "Generating lex.yy.c"
#)
#MESSAGE(STATUS ${CMAKE_SOURCE_DIR})
#set(flex_yacc_cmd "make clean; make ly")
#set(frontend_dir "src/frontend")
#execute_process(COMMAND ${flex_yacc_cmd} WORKING_DIRECTORY ${frontend_dir})
##execute_process(COMMAND ${flex_yacc_cmd_new} WORKING_DIRECTORY ${frontend_dir})

# all of C++ source files
file(GLOB_RECURSE SOURCES_C "src/*.c")
file(GLOB_RECURSE SOURCES_CXX "src/*.cpp" "src/*.cc")

set_source_files_properties(${SOURCES_C} PROPERTIES LANGUAGE CXX )

# executable
add_executable(frontend ${SOURCES_C} ${SOURCES_CXX} ${BISON_MyParser_OUTPUTS} src/frontend/main.cpp src/frontend/main.cpp src/define/ast.cpp)
