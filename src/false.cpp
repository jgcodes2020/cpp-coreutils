#include <cstdlib>
#include <iostream>
#include <string_view>

#include <fmt/core.h>

using namespace std::string_view_literals;

void usage(std::string_view argv0) {
  std::cout << fmt::format(R"msg(
usage: {0} [ignored]...
   or: {0} --help
Returns with an exit code indicating failure.

Options:
  --help  print this help page before exiting
  
NOTE: Some shells have false as a builtin command, which will likely override 
this one. Please check your shell's manual for information on its version.
)msg"sv.substr(1), argv0);
}

int main(int argc, char* argv[]) {
  if (argc == 2 && std::string_view(argv[1]) == "--help") {
    usage(argv[0]);
  }
  return EXIT_FAILURE;
}