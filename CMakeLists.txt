cmake_minimum_required(VERSION 2.8)

SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++-3.6")
SET ( LFLAGS=`llvm-config --cppflags --ldflags --libs core` )

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS} -std=c++11)
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


set(SOURCES
  toy.cpp
)

message(STATUS ${LLVM_LIBS} hej)


add_executable(toy ${SOURCES})

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(toy ${LLVM_LIBS})
