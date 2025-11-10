cmake_minimum_required(VERSION 3.19)

project(libdsp)

# Build libdsp as a shared library
add_library(libdsp SHARED libdsp.c)

# Set output name
set_target_properties(libdsp PROPERTIES
    OUTPUT_NAME "dsp"
    PREFIX "lib"
)

# Get the absolute path to the support directory (Max package structure)
get_filename_component(SUPPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../support" ABSOLUTE)

# Set output directory to support folder (standard Max package location for shared libraries)
# Override per-configuration subdirectories to output directly to support/
set_target_properties(libdsp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${SUPPORT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${SUPPORT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${SUPPORT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${SUPPORT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SUPPORT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SUPPORT_DIR}"
)

# Link with math library
target_link_libraries(libdsp m)
