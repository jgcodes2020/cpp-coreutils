project(cpp-coreutils
  DESCRIPTION "A reimplementation of some of the POSIX utilities in C++"
  LANGUAGES CXX
)

add_executable(echo
  src/echo.cpp
)