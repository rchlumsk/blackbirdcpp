cmake_minimum_required(VERSION 3.20)
# made with help from Zhu Liang-Jun of IGSNRR in Beijing
# modified for GitHub by Trevor James Smith of Ouranos in Montreal

# optional cmake command line arguments (e.g, "cmake -D COMPILE_EXE=ON" .)
option(COMPILE_EXE "If ON, will create a executable file (default: ON)" ON)

# Try to locate vcpkg automatically
if(NOT DEFINED VCPKG_ROOT)
  # Look for vcpkg.exe in PATH
  find_program(VCPKG_EXE NAMES vcpkg)
  if(VCPKG_EXE)
	get_filename_component(VCPKG_ROOT "${VCPKG_EXE}" DIRECTORY)
  else()
	message(FATAL_ERROR "Could not find vcpkg.exe. Please install vcpkg or set VCPKG_ROOT manually.")
  endif()
endif()

# Force vcpkg static libraries
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
if(WIN32)
  set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "" FORCE)
elseif(APPLE)
  if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
	set(VCPKG_TARGET_TRIPLET "arm64-osx-static" CACHE STRING "" FORCE)
  else()
	set(VCPKG_TARGET_TRIPLET "x64-osx-static" CACHE STRING "" FORCE)
  endif()
elseif(UNIX)
  set(VCPKG_TARGET_TRIPLET "x64-linux-static" CACHE STRING "" FORCE)
endif()
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")
set(CMAKE_FIND_ROOT_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}")
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Log message
message(STATUS "Using vcpkg root: ${VCPKG_ROOT}, triplet: ${VCPKG_TARGET_TRIPLET}")

# Setup Project
PROJECT(blackbird CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the cmake directory to the module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Find packages
#find_package(NetCDF CONFIG REQUIRED PATHS "${CMAKE_FIND_ROOT_PATH}" NO_DEFAULT_PATH)
find_package(GDAL CONFIG REQUIRED PATHS "${CMAKE_FIND_ROOT_PATH}" NO_DEFAULT_PATH)

# find header & source & resource
file(GLOB HEADER "src/*.h")
file(GLOB SOURCE "src/*.cpp")
if(WIN32)
  set(RESOURCE "src/blackbird.rc")
else()
  set(RESOURCE "")
endif()

# Set MSVC runtime
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
endif()

# creates an executable - file extension is OS dependent (Linux: none, Windows: .exe)
if(COMPILE_EXE)
  add_executable(blackbird
    ${SOURCE}
    ${HEADER}
    ${RESOURCE}
  )
  target_compile_definitions(blackbird PUBLIC STANDALONE)
  #target_link_libraries(blackbird PRIVATE netCDF::netcdf)
  target_link_libraries(blackbird PRIVATE GDAL::GDAL)
  set_target_properties(blackbird PROPERTIES LINKER_LANGUAGE CXX)
endif()
source_group("Header Files" FILES ${HEADER})
source_group("Source Files" FILES ${SOURCE})

# unset cmake variables to avoid polluting the cache
unset(COMPILE_EXE CACHE)
