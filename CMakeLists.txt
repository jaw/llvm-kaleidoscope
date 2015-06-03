cmake_minimum_required(VERSION 2.8)

SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++-3.6")
#SET ( LFLAGS=`llvm-config --cppflags --ldflags --libs core` )

find_package(LLVM REQUIRED CONFIG)

message(STATUS llvm definitions: 	${LLVM_DEFINITIONS} )

add_definitions(
	${LLVM_DEFINITIONS}
	-std=c++11
	-g
)
include_directories(${LLVM_INCLUDE_DIRS})

set(SOURCES
	ast.h
	lex.h
	parse.h
	debug.h
	codegen.h
	dispatch.h
  toy.cpp
)

add_executable(toy ${SOURCES})

llvm_map_components_to_libnames(llvm_libs core backend native codegen mcjit)

message(STATUS llvm libs: ${llvm_libs})

target_link_libraries(toy ${llvm_libs})
