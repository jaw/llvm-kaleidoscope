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
include_directories(
${CMAKE_SOURCE_DIR}/
${CMAKE_SOURCE_DIR}/vsxu
${CMAKE_SOURCE_DIR}/vsxu/container
${CMAKE_SOURCE_DIR}/vsxu/string
${CMAKE_SOURCE_DIR}/vsxu/debug
${CMAKE_SOURCE_DIR}/vsxu/tools
${LLVM_INCLUDE_DIRS}
)

set(SOURCES
	toy.cpp
	ast/ast.h
	ast/ast_abs.h
	ast/ast_parse.h
	ast/ast_expr.h
	binop_precedence.h
	error.h
	lex.h
	parse.h
	debuginfo/debuginfo_abs.h
	debuginfo/debuginfo.h
	debuginfo/debuginfo_manager.h
	debuginfo/debuginfo_manager.cpp
	codegen.h
	dispatch.h
	producer.h
	producer.cpp
	vsxu/string/vsx_string.h
)

add_executable(toy ${SOURCES})

llvm_map_components_to_libnames(llvm_libs core backend native codegen mcjit)

message(STATUS llvm libs: ${llvm_libs})

target_link_libraries(toy ${llvm_libs})
