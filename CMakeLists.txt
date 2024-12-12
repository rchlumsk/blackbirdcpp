cmake_minimum_required(VERSION 3.20)
# made with help from Zhu Liang-Jun of IGSNRR in Beijing
# modified for GitHub by Trevor James Smith of Ouranos in Montreal

# optional cmake command line arguments (e.g, "cmake -D COMPILE_LIB=ON" .)
option(COMPILE_EXE "If ON, will create a executable file (default: ON)" ON)

# Setup Project
PROJECT(blackbird CXX)

# Add the cmake directory to the module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# find header & source
file(GLOB HEADER "src/*.h")
file(GLOB SOURCE "src/*.cpp")

# creates an executable - file extension is OS dependent (Linux: none, Windows: .exe)
if(COMPILE_EXE)
  add_executable(blackbird
    ${SOURCE}
    ${HEADER}
  )
  target_compile_definitions(blackbird PUBLIC STANDALONE)
  set_target_properties(blackbird PROPERTIES LINKER_LANGUAGE CXX)
endif()
source_group("Header Files" FILES ${HEADER})
source_group("Source Files" FILES ${SOURCE})

# unset cmake variables to avoid polluting the cache
unset(COMPILE_LIB CACHE)
unset(COMPILE_EXE CACHE)
