#############################################################
# COMMON LIBRARY FOR LUAJIT-MAX EXTERNALS
#############################################################

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version" FORCE)

# Get LuaJIT paths (use system or local build based on parent config)
if(USE_SYSTEM_LUAJIT)
    set(LUAJIT_INCLUDE ${SYSTEM_LUAJIT_INCLUDE})
    set(LUAJIT_LIB ${SYSTEM_LUAJIT_LIB})
else()
    set(LUAJIT ${CMAKE_BINARY_DIR}/deps/luajit-install)
    set(LUAJIT_INCLUDE ${LUAJIT}/include/luajit-2.1)
    set(LUAJIT_LIB ${LUAJIT}/lib/libluajit-5.1.a)
endif()

# Get Max SDK includes (from parent scope)
set(MAX_SDK_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base)
set(MAX_SDK_INCLUDES "${MAX_SDK_BASE}/c74support/max-includes")
set(MAX_SDK_MSP_INCLUDES "${MAX_SDK_BASE}/c74support/msp-includes")

# Header-only library - single-header luajit_external.h
set(COMMON_HEADERS
    luajit_external.h
    luajit_api.h
)

# Create interface library (header-only)
add_library(luajit_common INTERFACE)

# Include directories for interface library
target_include_directories(luajit_common
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LUAJIT_INCLUDE}
    ${MAX_SDK_INCLUDES}
    ${MAX_SDK_MSP_INCLUDES}
)

# Link LuaJIT for interface library
target_link_libraries(luajit_common
    INTERFACE
    ${LUAJIT_LIB}
)

MESSAGE("Common library configured")
MESSAGE("  LUAJIT_INCLUDE: ${LUAJIT_INCLUDE}")
MESSAGE("  MAX_SDK_INCLUDES: ${MAX_SDK_INCLUDES}")
