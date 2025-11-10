#############################################################
# COMMON LIBRARY FOR LUAJIT-MAX EXTERNALS
#############################################################

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version" FORCE)

# Get LuaJIT paths
set(LUAJIT ${CMAKE_BINARY_DIR}/deps/luajit-install)
set(LUAJIT_INCLUDE ${LUAJIT}/include/luajit-2.1)
set(LUAJIT_LIB ${LUAJIT}/lib/libluajit-5.1.a)

# Get Max SDK includes (from parent scope)
set(MAX_SDK_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base)
set(MAX_SDK_INCLUDES "${MAX_SDK_BASE}/c74support/max-includes")
set(MAX_SDK_MSP_INCLUDES "${MAX_SDK_BASE}/c74support/msp-includes")

# Source files for common library
set(COMMON_SOURCES
    lua_engine.c
    max_helpers.c
)

set(COMMON_HEADERS
    lua_engine.h
    max_helpers.h
)

# Create static library
add_library(luajit_common STATIC
    ${COMMON_SOURCES}
    ${COMMON_HEADERS}
)

# Include directories
target_include_directories(luajit_common
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LUAJIT_INCLUDE}
    ${MAX_SDK_INCLUDES}
    ${MAX_SDK_MSP_INCLUDES}
)

# Link LuaJIT
target_link_libraries(luajit_common
    PUBLIC
    ${LUAJIT_LIB}
)

# Set C standard
set_target_properties(luajit_common PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

MESSAGE("Common library configured")
MESSAGE("  LUAJIT_INCLUDE: ${LUAJIT_INCLUDE}")
MESSAGE("  MAX_SDK_INCLUDES: ${MAX_SDK_INCLUDES}")
