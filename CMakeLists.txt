cmake_minimum_required(VERSION 3.19)

string(REGEX REPLACE "(.*)/" "" THIS_FOLDER_NAME "${CMAKE_CURRENT_SOURCE_DIR}")
project(${THIS_FOLDER_NAME})

option(C74_BUILD_FAT "Build Universal Externals")


if (APPLE)
    if (${CMAKE_GENERATOR} MATCHES "Xcode")
            if (${XCODE_VERSION} VERSION_LESS 10)
                message(STATUS "Xcode 10 or higher is required. Please install from the Mac App Store.")
                return ()
            # elseif(${XCODE_VERSION} VERSION_GREATER_EQUAL 12)
            #     set(C74_BUILD_FAT YES)
            endif ()
    endif ()

    if (NOT CMAKE_OSX_ARCHITECTURES)
        if(C74_BUILD_FAT)
            set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "macOS architecture" FORCE)
        else()
            set(CMAKE_OSX_ARCHITECTURES ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "macOS architecture" FORCE)
        endif()
        message("CMAKE_OSX_ARCHITECTURES set to ${CMAKE_OSX_ARCHITECTURES}")
    endif()
endif()

set(local_prefix "/usr/local")
if(CMAKE_HOST_APPLE) # i.e if is_macos check if homebrew is used aand if so get prefix
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE homebrew_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(DEFINED homebrew_prefix)
        set(local_prefix ${homebrew_prefix})
    endif()
endif()

# Option to force building LuaJIT from source even if system LuaJIT is available
option(FORCE_BUILD_LUAJIT "Force building LuaJIT from source" OFF)

# Check for system LuaJIT (Homebrew or other package manager)
set(USE_SYSTEM_LUAJIT OFF)
if(FORCE_BUILD_LUAJIT)
    message(STATUS "FORCE_BUILD_LUAJIT enabled, skipping system LuaJIT detection")
elseif(CMAKE_HOST_APPLE)
    execute_process(
        COMMAND brew --prefix luajit
        OUTPUT_VARIABLE LUAJIT_BREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE BREW_LUAJIT_RESULT
    )
    if(BREW_LUAJIT_RESULT EQUAL 0 AND EXISTS "${LUAJIT_BREW_PREFIX}/include/luajit-2.1")
        set(USE_SYSTEM_LUAJIT ON)
        set(SYSTEM_LUAJIT_PREFIX ${LUAJIT_BREW_PREFIX})
        set(SYSTEM_LUAJIT_INCLUDE ${LUAJIT_BREW_PREFIX}/include/luajit-2.1)
        set(SYSTEM_LUAJIT_LIB ${LUAJIT_BREW_PREFIX}/lib/libluajit-5.1.a)
        message(STATUS "Found system LuaJIT: ${LUAJIT_BREW_PREFIX}")
    endif()
endif()

if(USE_SYSTEM_LUAJIT)
    message(STATUS "Using system LuaJIT from: ${SYSTEM_LUAJIT_PREFIX}")
else()
    message(STATUS "Using built LuaJIT from: ${CMAKE_BINARY_DIR}/deps/luajit-install")
endif()

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

# Build common library first (shared by all externals)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/projects/common/CMakeLists.txt")
  message("Generating: common (shared library)")
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/common)
endif ()

# Generate a project for every folder in the "source/category" folder
SUBDIRLIST(CATEGORY_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source)
foreach (cat_dir ${CATEGORY_DIRS})
  SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/${cat_dir})
  foreach (project_dir ${PROJECT_DIRS})
    # Skip common directory since we already built it
    if (project_dir STREQUAL "common")
      continue()
    endif ()

    set(project_path ${CMAKE_CURRENT_SOURCE_DIR}/source/${cat_dir}/${project_dir})
    message("checking in: ${project_path}")
    if (EXISTS "${project_path}/CMakeLists.txt")
      message("Generating: ${project_dir}")
      add_subdirectory(${project_path})
    endif ()
  endforeach ()
endforeach ()


