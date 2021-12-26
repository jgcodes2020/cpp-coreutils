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

using args_t   = std::vector<std::string>;

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

int main(int argc, char* argv[]) {
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
        std::cerr << fmt::format("{}: internal error. Backtrace: \n", ext_argv[0]);
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
  
  if (args.size() == 0) return 1;
  
  return int(!eval_logic(eval_conditions(args)));
}