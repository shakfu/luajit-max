include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)
set(CMAKE_VERBOSE_MAKEFILE ON)

#############################################################
# MAX EXTERNAL
#############################################################
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version" FORCE)

# Use system LuaJIT if available, otherwise use built version
if(USE_SYSTEM_LUAJIT)
    set(LUAJIT_INCLUDE ${SYSTEM_LUAJIT_INCLUDE})
    set(LUAJIT_LIB ${SYSTEM_LUAJIT_LIB})
    MESSAGE("Using system LUAJIT_INCLUDE: ${LUAJIT_INCLUDE}")
    MESSAGE("Using system LUAJIT_LIB: ${LUAJIT_LIB}")
else()
    set(LUAJIT ${CMAKE_BINARY_DIR}/deps/luajit-install)
    set(LUAJIT_INCLUDE ${LUAJIT}/include/luajit-2.1)
    set(LUAJIT_LIB ${LUAJIT}/lib/libluajit-5.1.a)
    MESSAGE("Using built LUAJIT_INCLUDE: ${LUAJIT_INCLUDE}")
    MESSAGE("Using built LUAJIT_LIB: ${LUAJIT_LIB}")
endif()

include_directories( 
  "${MAX_SDK_INCLUDES}"
  "${MAX_SDK_MSP_INCLUDES}"
  "${MAX_SDK_JIT_INCLUDES}"
)

file(GLOB PROJECT_SRC
   "*.h"
   "*.c"
   "*.cpp"
)

add_library( 
    ${PROJECT_NAME} 
    MODULE
    ${PROJECT_SRC}
)


target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${LUAJIT_INCLUDE}
    ${CMAKE_CURRENT_SOURCE_DIR}/../libapi
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)


target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ${LUAJIT_LIB}
    luajit_common
)



include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
