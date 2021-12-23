#include <cstdlib>
#include <iostream>
#include <string_view>

using namespace std::string_view_literals;

void usage() {
  std::cout << R"msg(
usage: false [ignored]...
   or: false --help
Returns with an exit code indicating failure.

Options:
  --help  print this help message before exiting
  
NOTE: Some shells have false as a builtin command, which will likely override 
this one. Please check your shell's manual for information on its version.
)msg"sv.substr(1);
}

int main(int argc, char* argv[]) {
  if (argc == 2 && std::string_view(argv[1]) == "--help") {
    usage();
  }
  return EXIT_FAILURE;
}