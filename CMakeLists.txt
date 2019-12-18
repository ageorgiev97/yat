cmake_minimum_required (VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	set(
        CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
		CACHE STRING ""
    )
	message(" [INFO] VCPKG CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
endif()

project(yat)

find_package(LLVM REQUIRED)


file(
    GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)

set(PARSER_Y ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.y)
set(LEXER_L  ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.l)

set(PARSER_H ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp)
set(PARSER_C ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)

set(LEXER_C ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

set(BISON F:/Daniel/flexbison/bison.exe)
set(FLEX F:/Daniel/flexbison/flex.exe)

add_custom_target(
    Parser ALL
    BYPRODUCTS ${PARSER_H} ${PARSER_C}
    COMMAND ${BISON} --defines=${PARSER_H} --output=${PARSER_C} ${PARSER_Y}
)

add_custom_target(
    Lexer ALL
    BYPRODUCTS ${LEXER_C}
    COMMAND ${FLEX} --wincompat --outfile=${LEXER_C} ${LEXER_L}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /CGTHREADS:8 /INCREMENTAL:NO")

set(CMAKE_CXX_FLAGS_DEBUG "/JMC /Od /Z7 /MDd")

set(CMAKE_CXX_FLAGS_RELEASE "/O2 /GS- /guard:cf /D NDEBUG /JMC- /GL /Ob3 /MD")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/OPT:REF /OPT:ICF=10000 /LTCG")

set(CMAKE_CXX_STANDARD 17)

add_executable(yat ${LEXER_C} ${PARSER_H} ${PARSER_C} ${SOURCES})
add_dependencies(yat Parser Lexer)

target_include_directories(yat PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs analysis instcombine object runtimedyld scalaropts support native support core irreader executionengine mcjit mc mcdisassembler)
target_link_libraries(yat ${llvm_libs})