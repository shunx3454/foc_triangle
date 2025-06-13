cmake_minimum_required(VERSION 3.22)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME FOC_Triangle)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")
include("cmake/at32f403a.cmake")
include("cmake/middlewares.cmake")
include("cmake/cmsis_dsp.cmake")
include("cmake/bsp.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})

# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add some compile commond
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    -Wno-unused-parameter
    -Wno-pedantic
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add user defined sources
file(GLOB USER_SRC 
    user/src/*.c
    user/src/*.cpp
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${AT32F403A_DRIVER_SRC}
    ${MIDDLEWARES_SRC}
    ${CMSIS_DSP_SRC}
    ${BSP_SRC}
    ${USER_SRC}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${MIDDLEWARES_INC}
    ${AT32F403A_DRIVER_INC}
    ${CMSIS_DSP_INC}
    ${BSP_INC}
    user/inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
	AT32F403AVGT7
    USE_STDPERIPH_DRIVER
    # CortexM4 FPU
    ARM_MATH_CM4
    ARM_MATH_MATRIX_CHECK
    ARM_MATH_ROUNDING
    UNALIGNED_SUPPORT_DISABLE
    # Debug
    $<$<CONFIG:Debug>:DEBUG>
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    # Add user defined libraries
)

# Generate HEX and BIN file
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)
set(DSI_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.dsi)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMAND echo "Building ${HEX_FILE}"
    COMMAND echo "Building ${BIN_FILE}"
    # COMMAND echo "Building ${DSI_FILE}"
    COMMAND ${CMAKE_OBJCOPY} -Oihex   ${PROJECT_NAME}.elf ${HEX_FILE}
    COMMAND ${CMAKE_OBJCOPY} -Obinary ${PROJECT_NAME}.elf ${BIN_FILE}
    # COMMAND ${CMAKE_OBJDUMP} -D -S    ${PROJECT_NAME}.elf > ${DSI_FILE}
)