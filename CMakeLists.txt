##########################################################################################
#                                                                                        #
# OpenSpace                                                                              #
#                                                                                        #
# Copyright (c) 2014-2021                                                                #
#                                                                                        #
# Permission is hereby granted, free of charge, to any person obtaining a copy of this   #
# software and associated documentation files (the "Software"), to deal in the Software  #
# without restriction, including without limitation the rights to use, copy, modify,     #
# merge, publish, distribute, sublicense, and/or sell copies of the Software, and to     #
# permit persons to whom the Software is furnished to do so, subject to the following    #
# conditions:                                                                            #
#                                                                                        #
# The above copyright notice and this permission notice shall be included in all copies  #
# or substantial portions of the Software.                                               #
#                                                                                        #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,    #
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A          #
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT     #
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF   #
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE   #
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                          #
##########################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(OpenSpace)

set(OPENSPACE_VERSION_MAJOR 0)
set(OPENSPACE_VERSION_MINOR 16)
set(OPENSPACE_VERSION_PATCH 0)
set(OPENSPACE_VERSION_STRING "Beta-8")

set(OPENSPACE_BASE_DIR "${PROJECT_SOURCE_DIR}")
set(OPENSPACE_CMAKE_EXT_DIR "${OPENSPACE_BASE_DIR}/support/cmake")
set(GHOUL_BASE_DIR "${OPENSPACE_BASE_DIR}/ext/ghoul")

include(${OPENSPACE_CMAKE_EXT_DIR}/module_common.cmake)
include(${OPENSPACE_CMAKE_EXT_DIR}/global_variables.cmake)
include(${GHOUL_BASE_DIR}/support/cmake/copy_shared_libraries.cmake)
include(${GHOUL_BASE_DIR}/support/cmake/handle_external_library.cmake)
include(${GHOUL_BASE_DIR}/support/cmake/message_macros.cmake)

begin_header("Configuring OpenSpace project")

# Bail out if the user tries to generate a 32 bit project.
if (NOT ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  message(FATAL_ERROR "OpenSpace can only be generated for 64 bit architectures.")
endif ()

##########################################################################################
# Cleanup project                                                                        #
##########################################################################################
set(OPENSPACE_APPS_DIR "${OPENSPACE_BASE_DIR}/apps")

if (NOT EXISTS "${OPENSPACE_BASE_DIR}/ext/ghoul/CMakeLists.txt")
  message(FATAL_ERROR "Git submodules are missing. Please run "
    "git submodule update --init --recursive to download the missing dependencies."
  )
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER CMake)

mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY CMAKE_BUILD_TYPE CMAKE_DEBUG_POSTFIX
  CMAKE_INSTALL_PREFIX CMAKE_OSX_ARCHITECTURES CMAKE_OSX_DEPLOYMENT_TARGET
  CMAKE_OSX_SYSROOT CMAKE_RELEASE_POSTFIX
)

# Set build output directories
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${OPENSPACE_CMAKE_EXT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OPENSPACE_BASE_DIR}/bin)

# "OpenSpace Helper" is not a valid CMake target name under OLD
cmake_policy(SET CMP0037 NEW)

##########################################################################################
# Main                                                                                   #
##########################################################################################

# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE OPENSPACE_GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE OPENSPACE_GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# See if working directory is clean or not
execute_process(
  COMMAND git diff-index --quiet HEAD --
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE OPENSPACE_GIT_STATUS_RETURN
)
if (NOT OPENSPACE_GIT_STATUS_RETURN EQUAL 0)
  set(OPENSPACE_GIT_STATUS "uncommitted changes")
else ()
  set(OPENSPACE_GIT_STATUS "")
endif ()

option(OPENSPACE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

if (MSVC)
  option(OPENSPACE_OPTIMIZATION_ENABLE_AVX "Enable AVX instruction set for compilation" OFF)
  option(OPENSPACE_OPTIMIZATION_ENABLE_AVX2 "Enable AVX2 instruction set for compilation" OFF)
  option(OPENSPACE_OPTIMIZATION_ENABLE_AVX512 "Enable AVX2 instruction set for compilation" OFF)
  option(OPENSPACE_OPTIMIZATION_ENABLE_OTHER_OPTIMIZATIONS "Enable other optimizations, like LTCG, intrinsics, etc")

  if (OPENSPACE_OPTIMIZATION_ENABLE_AVX AND OPENSPACE_OPTIMIZATION_ENABLE_AVX2)
    message(FATAL_ERROR "Cannot enable AVX and AVX2 instructions simultaneously")
  endif ()

  if (OPENSPACE_OPTIMIZATION_ENABLE_AVX AND OPENSPACE_OPTIMIZATION_ENABLE_AVX512)
    message(FATAL_ERROR "Cannot enable AVX and AVX512 instructions simultaneously")
  endif ()

  if (OPENSPACE_OPTIMIZATION_ENABLE_AVX2 AND OPENSPACE_OPTIMIZATION_ENABLE_AVX512)
    message(FATAL_ERROR "Cannot enable AVX2 and AVX512 instructions simultaneously")
  endif ()

  set(GHOUL_OPTIMIZATION_ENABLE_AVX ${OPENSPACE_OPTIMIZATION_ENABLE_AVX} CACHE BOOL "" FORCE)
  set(GHOUL_OPTIMIZATION_ENABLE_AVX2 ${OPENSPACE_OPTIMIZATION_ENABLE_AVX2} CACHE BOOL "" FORCE)
  set(GHOUL_OPTIMIZATION_ENABLE_AVX512 ${OPENSPACE_OPTIMIZATION_ENABLE_AVX512} CACHE BOOL "" FORCE)
  set(GHOUL_OPTIMIZATION_ENABLE_OTHER_OPTIMIZATIONS ${OPENSPACE_OPTIMIZATION_ENABLE_OTHER_OPTIMIZATIONS} CACHE BOOL "" FORCE)
endif ()

option(OPENSPACE_WITH_ABUFFER_RENDERER "Compile ABuffer Renderer" OFF)

if (UNIX AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++ -lc++abi")
endif ()

add_subdirectory(ext)

add_subdirectory(src)

add_subdirectory(support/coding/codegen)
# It is important that the __codegen.h do not actually exist so
# that this target is never considered as finished
add_custom_target(
  run_codegen
  ALL DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/__codegen.h"
)
add_dependencies(run_codegen codegen)
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/__codegen.h"
  COMMAND codegen ARGS "${OPENSPACE_BASE_DIR}/modules" "${OPENSPACE_BASE_DIR}/src"
  VERBATIM
)
set_folder_location(codegen-lib "support")
set_folder_location(codegen "support")
set_folder_location(run_codegen "support")


# Qt
# Unfortunately, we have to set this value manually; sigh
# In the future, if the Qt version is updated, just add to this variable ---abock
if (APPLE)
  set(CMAKE_PREFIX_PATH
    "~/Qt/5.6/clang_64/lib/cmake"
    "~/Qt/5.7/clang_64/lib/cmake"
    "~/Qt/5.8/clang_64/lib/cmake"
    "~/Qt/5.9/clang_64/lib/cmake"
    "~/Qt/5.10/clang_64/lib/cmake"
    "~/Qt/5.11/clang_64/lib/cmake"
    "~/Qt/5.12/clang_64/lib/cmake"
    "~/Qt/5.15.1/clang_64/lib/cmake"
  )
endif ()

if (MSVC)
  option(OPENSPACE_NVTOOLS_ENABLED "Include support for Nvidia Tools Extensions" OFF)
  set(OPENSPACE_NVTOOLS_PATH "C:/Program Files/NVIDIA Corporation/NvToolsExt")
  if (OPENSPACE_NVTOOLS_ENABLED)
    begin_dependency("Nvidia Tools Extension")
    target_compile_definitions(openspace-core PUBLIC "OPENSPACE_HAS_NVTOOLS")
    target_include_directories(openspace-core PUBLIC "${OPENSPACE_NVTOOLS_PATH}/include")
    end_dependency()
  endif ()
endif ()

begin_header("Configuring Modules")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/modules")
end_header("End: Configuring Modules")


add_subdirectory(support/coding/codegen/tests)
set_folder_location(run_test_codegen "Unit Tests/support")
set_folder_location(codegentest "Unit Tests")


begin_header("Configuring Applications")
add_subdirectory("${OPENSPACE_APPS_DIR}")
end_header("End: Configuring Applications")


if (MSVC AND TARGET OpenSpace)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenSpace)
endif()


option(OPENSPACE_HAVE_TESTS "Activate the OpenSpace unit tests" ON)
if (OPENSPACE_HAVE_TESTS)
  begin_header("Generating OpenSpace unit test")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")
  end_header()
endif (OPENSPACE_HAVE_TESTS)


# Web Browser and Web gui
# Why not put these in the module's path? Because they do not have access to the
# target as of July 2017, which is needed.
if (OPENSPACE_MODULE_WEBBROWSER AND CEF_ROOT)
  # wanted by CEF
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "CMAKE_BUILD_TYPE")

  set(PROJECT_ARCH "x86_64")

  if (WIN32)
    set(RESOURCE_FILE ${OPENSPACE_APPS_DIR}/OpenSpace/openspace.rc)
  endif ()

  # Add the CEF binary distribution's cmake/ directory to the module path and
  # find CEF to initialize it properly.
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${WEBBROWSER_MODULE_PATH}/cmake")
  include(webbrowser_helpers)
elseif ()
  message(WARNING "Web configured to be included, but no CEF_ROOT was found, please try configuring CMake again")
endif ()

##########################################################################################
# Misc settings                                                                          #
##########################################################################################

# Manage the CPack packaging
include(${OPENSPACE_CMAKE_EXT_DIR}/packaging.cmake)

end_header("End: Configuring OpenSpace project")
