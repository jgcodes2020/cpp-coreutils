#include <iostream>
#include <iterator>
#include <string_view>

#include <fmt/core.h>
#include <cxxopts.hpp>

void usage(std::string_view argv0) {
  using namespace std::string_view_literals;
  fmt::format_to(std::ostreambuf_iterator(std::cout), R"msg(
usage: ls [OPTIONS...] [FILES...]

Lists the files/directories in FILES. If no FILES are specified, lists the current directory.

Options:
  -A      do not ignore entries beginning with .
  -a      Like -A, but list implied . and .. as well
  -C      write output in columns, sorted vertically. disables long-form output.
  -c      use 'last file status change' instead of 'last modified' for [-l, -t]
  -d      do not list directory contents
  -F      add indicator chars after each filename
  -f      don't sort, enable -a
  -g      don't write file owner info (enables -l)
  -H      dereference symlinks on the command line
  -i      list file serial numbers in addition to the info being presented
  -k      write out file sizes in kb
  -L      dereference all symlinks
  -l      write output in long form
  -m      list directories using a comma-separated list
  -n      write user/group id instead of name (enables -l)
  -o      don't write file group info (enables -l)
  -p      like -f, but only for directories
  -q      replace non-printable characters with ?
  -R      recursively list all subdirectories
  -r      for options [-s, -t], reverse order of sorting
  -S      sort entries by file size, largest first
  -s      display file size
  -t      sort entries by time modified, newest first
  -u      use 'last accessed' instead of 'last modified' for [-l, -t]
  -x      write output in columns, sorted horizontally
  -1      force output as one entry per line
  
  --help  print this help page and exit
)msg"sv.substr(1), argv0);
}

int main(const int argc, const char* argv[]) {
  using namespace std::string_view_literals;
  
  cxxopts::Options parser("ls");
  // clang-format off
  parser.add_options()
    ("A", "")("C", "")("F", "")
    ("H", "")("L", "")("R", "")
    ("S", "")("a", "")("c", "")
    ("d", "")("f", "")("g", "")
    ("i", "")("k", "")("l", "")
    ("m", "")("n", "")("o", "")
    ("p", "")("q", "")("r", "")
    ("s", "")("t", "")("u", "")
    ("x", "")("1", "")
    ("help", "");
  // clang-format on
  auto args = parser.parse(argc, argv);
  
  if (args.count("help")) {
    usage(argv[0]);
    return 0;
  }
}