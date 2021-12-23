cmake_minimum_required(VERSION 3.15)
project(cpp-coreutils
  DESCRIPTION "A reimplementation of some of the POSIX utilities in C++"
  LANGUAGES CXX
)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_executable(echo
  src/echo.cpp
)
set_target_properties(echo PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED yes
)
target_compile_features(echo PUBLIC cxx_std_17)
target_link_libraries(echo CONAN_PKG::cxxopts)

set(CMAKE_EXPORT_COMPILE_COMMANDS yes)