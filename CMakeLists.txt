cmake_minimum_required(VERSION 3.15)
project(cxx-coreutils
  DESCRIPTION "A reimplementation of some of the POSIX utilities in C++"
  LANGUAGES CXX
)

include(cmake/conan.cmake)
conan_cmake_autodetect(conan_settings)
conan_cmake_install(
  PATH_OR_REFERENCE ${CMAKE_CURRENT_SOURCE_DIR}
  BUILD missing
  SETTINGS ${conan_settings}
  INSTALL_FOLDER ${CMAKE_BINARY_DIR}/CMakeFiles/conan_deps
)

# Required for conan generator
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/CMakeFiles/conan_deps)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/CMakeFiles/conan_deps)

foreach(pkg cxxopts fmt Microsoft.GSL)
  find_package(${pkg} REQUIRED)
endforeach()

macro(cxx_coreutils_setup target)
  set_target_properties(${target} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED yes
  )
  target_compile_features(${target} PUBLIC cxx_std_17)
  target_link_libraries(${target} cxxopts::cxxopts fmt::fmt)
endmacro()

# Basic logic
add_executable(echo src/echo.cpp)
cxx_coreutils_setup(echo)

add_executable("true" src/true.cpp)
cxx_coreutils_setup("true")

add_executable("false" src/false.cpp)
cxx_coreutils_setup("false")

add_executable(test
  src/test.cpp 
  src/details/test_helpers.cpp 
  src/details/test_helpers.hpp 
  src/details/parse_error.hpp
)
cxx_coreutils_setup(test)

get_target_property(coreutils_test_srcs test SOURCES)
add_executable(test_lbracket
  ${coreutils_test_srcs}
)
set_target_properties(test_lbracket PROPERTIES
  OUTPUT_NAME "["
)
target_compile_definitions(test_lbracket PUBLIC LBRACKET)
cxx_coreutils_setup(test_lbracket)

set(CMAKE_EXPORT_COMPILE_COMMANDS yes)