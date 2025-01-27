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

# Set the path to the license file
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSES/LICENSE.TXT")

# Include CPack module
include(CPack)

# Check if Capstone is installed, and if not, install it
find_package(Capstone)

if (NOT Capstone_FOUND)
    include(FetchContent)
    FetchContent_Declare(
            capstone
            GIT_REPOSITORY https://github.com/capstone-engine/capstone.git
            GIT_TAG next
    )
    # Set Capstone options before making it available
    set(FETCHCONTENT_QUIET FALSE)
    set(CAPSTONE_BUILD_STATIC ON)
    set(CAPSTONE_BUILD_SHARED OFF)
    set(CAPSTONE_BUILD_TESTS OFF)
    set(CAPSTONE_BUILD_CSTOOL OFF)
    set(CAPSTONE_ARM_SUPPORT OFF)
    set(CAPSTONE_ARM64_SUPPORT OFF)
    set(CAPSTONE_M68K_SUPPORT OFF)
    set(CAPSTONE_MIPS_SUPPORT OFF)
    set(CAPSTONE_PPC_SUPPORT OFF)
    set(CAPSTONE_SPARC_SUPPORT OFF)
    set(CAPSTONE_SYSZ_SUPPORT OFF)
    set(CAPSTONE_XCORE_SUPPORT OFF)
    set(CAPSTONE_TMS320C64X_SUPPORT OFF)
    set(CAPSTONE_M680X_SUPPORT OFF)
    set(CAPSTONE_EVM_SUPPORT OFF)
    set(CAPSTONE_WASM_SUPPORT OFF)
    set(CAPSTONE_BPF_SUPPORT OFF)
    set(CAPSTONE_RISCV_SUPPORT OFF)
    set(CAPSTONE_X86_SUPPORT ON)
    FetchContent_MakeAvailable(capstone)
    set(Capstone_INCLUDE_DIRS ${capstone_SOURCE_DIR}/include)
    set(Capstone_LIBRARIES capstone)
endif()

# Add the Capstone include directory
include_directories(${Capstone_INCLUDE_DIRS})

# Add the Capstone library directory
link_directories(${Capstone_LIBRARIES})

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

# Link linenoise and capstone with the main executable
target_link_libraries(dbgeko PRIVATE linenoise capstone)

# Add the helloworld example
add_executable(helloworld examples/helloworld.c)
target_link_libraries(helloworld PRIVATE linenoise)
target_link_options(helloworld PRIVATE -static)
set_target_properties(helloworld PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLES_OUTPUT_DIR})

# Set the assembler
enable_language(ASM_NASM)

# Set assembler flags
set(CMAKE_ASM_NASM_FLAGS "-f elf64")

# Ensure the output directory exists before building the assembly example
add_custom_target(create_obj_dir ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OBJECTS_OUTPUT_DIR}
)

# Add the assembly example
add_custom_target(helloworld_asm ALL
        COMMAND nasm -f elf64 -o ${OBJECTS_OUTPUT_DIR}/helloworld_asm.o ${CMAKE_SOURCE_DIR}/examples/helloworld.asm
        COMMAND ld -o ${EXAMPLES_OUTPUT_DIR}/helloworld_asm ${OBJECTS_OUTPUT_DIR}/helloworld_asm.o
        COMMENT "Assembling and linking assembly with ld"
        SOURCES examples/helloworld.asm
        DEPENDS create_obj_dir
)

# Add debugging flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")