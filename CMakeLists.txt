cmake_minimum_required(VERSION 3.15)
project(cxx-coreutils
  DESCRIPTION "A reimplementation of some of the POSIX utilities in C++"
  LANGUAGES CXX
)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

macro(cxx_coreutils_setup target)
  set_target_properties(${target} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED yes
  )
  target_compile_features(${target} PUBLIC cxx_std_17)
  target_link_libraries(${target} CONAN_PKG::cxxopts)
endmacro()

add_executable(echo src/echo.cpp)
cxx_coreutils_setup(echo)

add_executable(true src/true.cpp)
cxx_coreutils_setup(true)

add_executable(false src/false.cpp)
cxx_coreutils_setup(false)

set(CMAKE_EXPORT_COMPILE_COMMANDS yes)