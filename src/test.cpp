#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include <fmt/core.h>

#include "details/demangle.hpp"
#include "details/parse_error.hpp"
#include "details/test_helpers.hpp"

using namespace std::string_view_literals;

#ifdef LBRACKET
  #define COREUTILS_IS_LBRACKET true
#else
  #define COREUTILS_IS_LBRACKET false
#endif

using args_t = std::vector<std::string>;

void print_exception(const std::exception& e, size_t n = 0) {
  std::cerr << fmt::format(
    "{0:<3d} | {1}: {2}\n", n, coreutils::pretty_name(typeid(e)), e.what());
  try {
    std::rethrow_if_nested(e);
  }
  catch (const std::exception& exc) {
    print_exception(e, n + 1);
  }
}

static char** ext_argv;

void usage() {
  std::cout << fmt::format(R"msg(
Usage: test <predicate>
   OR: [ <predicate> ]
   OR: [ --help
Evaluates a predicate.

Predicates may be combined using the following operators:
! [P1]          Inverts the result of P1.
( [P1] )        Groups the expression inside the parentheses.
[P1] -a [P2]    Takes the logical AND of both conditions.
[P1] -o [P2]    Takes the logical OR of both conditions.

Base predicates
===================

Files:
-b [FILE]         true if FILE exists and is a block special device
-c [FILE]         true if FILE exists and is a character special device
-d [FILE]         true if FILE exists and is a directory
-e [FILE]         true if FILE exists
-f [FILE]         true if FILE exists and is a regular file
-h [FILE]         true if FILE exists and is a symbolic link
-L [FILE]         same as -h
-p [FILE]         true if FILE exists and is a FIFO (named pipe)

File permissions:
-r [FILE]         true if FILE can be read from
-w [FILE]         true if FILE can be written to
-x [FILE]         true if FILE can be executed as a program
-g [FILE]         true if FILE exists and uses set group ID (GID)
-u [FILE]         true if FILE exists and uses set user ID (UID)

File descriptors:
-t [FD]           true if FD is open and points to a terminal (isatty)

Strings:
-n [STR]          true if STR is not the empty string
-z [STR]          true if STR is the empty string
[STR]             same as -n. Prefer to use -n [STR] to avoid triggering --help.
[STR1] = [STR2]   true if STR1 and STR2 are equal
[STR1] != [STR2]  true if STR1 and STR2 are not equal

Integers:
[N1] -eq [N2]     true if N1 and N2 are equal
[N1] -ne [N2]     true if N1 and N2 are not equal
[N1] -gt [N2]     true if N1 is greater than N2
[N1] -ge [N2]     true if N1 is greater than or equal to N2
[N1] -lt [N2]     true if N1 is less than N2
[N1] -le [N2]     true if N1 is less than or equal to N2

Return value
============
0 if the test succeeded.
1 if the test failed.
2 if there was a syntax error.
3 otherwise.

NOTE 1: all integer comparisons are done using {1}signed long{0}. Trying to use an
integer that is too big will result in an error.
NOTE 2: Some shells have {1}test{0} and {1}[{0} as a builtin command, which will likely override 
this one. Please check your shell's manual for information on its version.
)msg"sv.substr(1), "\e[0m"sv, "\e[1m"sv);
}

int main(int argc, char* argv[]) {
  if constexpr (COREUTILS_IS_LBRACKET)
    if (argc == 2 && argv[1] == "--help"sv) {
      usage();
      return 0;
    }

  ext_argv = argv;
  std::set_terminate([] {
    auto exc = std::current_exception();
    if (exc) {
      try {
        std::rethrow_exception(exc);
      }
      catch (const coreutils::parse_error& e) {
        std::cerr << fmt::format("{}: parse error. Backtrace: \n", ext_argv[0]);
        print_exception(e);
        exit(2);
      }
      catch (const std::exception& e) {
        std::cerr << fmt::format(
          "{}: internal error. Backtrace: \n", ext_argv[0]);
        print_exception(e);
        exit(3);
      }
    }
    else {
      std::cerr << "std::terminate() called without exception\n";
    }
  });

  const args_t args = [argc, argv] {
    if (argc > 1) {
      if constexpr (COREUTILS_IS_LBRACKET) {
        if (std::string_view(argv[argc - 1]) != "]"sv) {
          throw coreutils::parse_error("Last argument of [ must be ]");
        }
        return args_t(argv + 1, argv + (argc - 1));
      }
      else {
        return args_t(argv + 1, argv + argc);
      }
    }
    else {
      return args_t();
    }
  }();

  using namespace coreutils::test;

  if (args.size() == 0)
    return 1;

  return int(!eval_logic(eval_conditions(args)));
}