cmake_minimum_required(VERSION 2.0)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

option(BUILD_CONFIG "Config: ci|coverage|release|pedantic" "ci")

# Set compiler build options

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Wall -pthread -fexceptions")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -ggdb -Wall -pthread -fexceptions")

set(DEBUG_FLAGS "-DDEBUG -O0 -fno-inline -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(RELEASE_FLAGS "-DNDEBUG -O3")

if("${BUILD_CONFIG}" STREQUAL "release")
  message("-- Prepare for Release build")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RELEASE_FLAGS}")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${RELEASE_FLAGS}")
elseif("${BUILD_CONFIG}" STREQUAL "coverage")
  message("-- Prepare for Code Coverage build")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS} --coverage")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${DEBUG_FLAGS} --coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
elseif("${BUILD_CONFIG}" STREQUAL "pedantic")
  message("-- Prepare for Pedantic build...")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -Weffc++ -Wextra -Wshadow -Wfloat-equal -Wundef -Wpointer-arith -Wcast-align -Wcast-qual -Wstrict-prototypes -Wstrict-overflow=3 -Wwrite-strings -Wswitch-enum -Winit-self -Wformat=2 -Wlogical-op -Wredundant-decls -Wconversion")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -O1 -Wconversion")
else()
  message("-- Prepare for Continuous Integration build...")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS} -Werror")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${DEBUG_FLAGS} -Werror")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat=2 -std=gnu++0x")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wformat=2")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -std=c++11")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wno-deprecated")
endif()

add_executable(rwg
  src/file/Deflate.cpp
  src/file/Deflate.h
  src/file/FileSystem.cpp
  src/file/FileSystem.h
  src/file/Chunk.cpp
  src/file/Chunk.h
  src/main.cpp
)
