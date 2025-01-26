cmake_minimum_required(VERSION 3.28)
project(dbgeko)

set(CMAKE_C_STANDARD 11)

# Set global output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set output directories for examples
set(EXAMPLES_OUTPUT_DIR ${CMAKE_BINARY_DIR}/examples)
set(OBJECTS_OUTPUT_DIR ${CMAKE_BINARY_DIR}/obj)

# Add the main executable
add_executable(dbgeko dbgeko.c utils.c utils.h dbglib.c dbglib.h)

# FetchContent module to download and include linenoise
include(FetchContent)
FetchContent_Declare(
        linenoise
        GIT_REPOSITORY https://github.com/antirez/linenoise.git
        GIT_TAG master
)
FetchContent_MakeAvailable(linenoise)

# Add linenoise library
add_library(linenoise STATIC ${linenoise_SOURCE_DIR}/linenoise.c)
target_include_directories(linenoise PUBLIC ${linenoise_SOURCE_DIR})

# Link linenoise with the main executable
target_link_libraries(dbgeko linenoise)

# Add the helloworld example
add_executable(helloworld examples/helloworld.c)
target_link_libraries(helloworld linenoise)
target_link_options(helloworld PRIVATE -static)
set_target_properties(helloworld PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLES_OUTPUT_DIR})

# Set the assembler
enable_language(ASM_NASM)

# Set assembler flags
set(CMAKE_ASM_NASM_FLAGS "-f elf64")

# Add the assembly example
add_custom_target(helloworld_asm ALL
        COMMAND nasm -f elf64 -o ${OBJECTS_OUTPUT_DIR}/helloworld_asm.o ${CMAKE_SOURCE_DIR}/examples/helloworld.asm
        COMMAND ld -o ${EXAMPLES_OUTPUT_DIR}/helloworld_asm ${OBJECTS_OUTPUT_DIR}/helloworld_asm.o
        COMMENT "Assembling and linking assembly with ld"
        SOURCES examples/helloworld.asm
)

# Add debugging flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")