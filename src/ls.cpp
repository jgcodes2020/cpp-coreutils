#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>

#include <fmt/core.h>

#include <mtap/mtap.hpp>

namespace fs = std::filesystem;

void usage(std::string_view argv0) {
  using namespace std::string_view_literals;
  fmt::format_to(
    std::ostreambuf_iterator(std::cout), R"msg(
usage: ls [OPTIONS...] [FILES...]

Lists the files/directories in FILES. If no FILES are specified, lists the current directory.

Options:
  -A      do not ignore entries beginning with .
  -a      Like -A, but list implied . and .. as well
  -C      write output in columns, sorted vertically
  -c      use 'last file status change' instead of 'last modified' for [-l, -t]
  -d      do not list directory contents
  -F      add indicator chars after each filename
  -f      don't sort, enable -a
  -g      don't write file owner info (enables -l)
  -H      dereference symlinks specified as arguments
  -i      list file serial numbers in addition to the info being presented
  -k      write out file sizes in KiB
  -L      dereference symlinks that are listed
  -l      write output in long form
  -m      list directories using a comma-separated list
  -n      write user/group id instead of name (enables -l)
  -o      don't write file group info (enables -l)
  -p      like -F, but only for directories
  -q      replace non-printable characters with ?
  -R      recursively list all subdirectories
  -r      for options [-s, -t], reverse order of sorting
  -S      sort entries by file size, largest first
  -s      display file size
  -t      sort entries by time modified, newest first
  -u      use 'last accessed' instead of 'last modified' for [-l, -t]
  -x      write output in columns, sorted horizontally
  -1      write output line-by-line
  
  --help  print this help page and exit
)msg"sv.substr(1),
    argv0);
}

struct option_data {
  enum class format { lines, columns_v, columns_h, details, csv };
  enum class time_src { last_modified, last_accessed, last_stat_change };
  enum class list_values { basic, list_hidden, list_all };
  enum class indicators { none, slash, all };
  enum class sort_key { name, none, time, size };
  enum class resolve_links { none, specified, listed };
  enum class escape_chars { none, qmark, cstyle };

  std::vector<fs::path> paths;

  format format          = format::lines;
  indicators indicators  = indicators::none;
  escape_chars esc_chars = escape_chars::none;

  // list content options
  list_values contents   = list_values::basic;
  resolve_links link_bhv = resolve_links::none;

  // list content flags
  bool list_dir_contents : 1 = true;
  bool recursive : 1         = false;

  // detail flags
  bool print_user : 1   = true;
  bool print_group : 1  = true;
  bool print_ids : 1    = false;
  bool print_size : 1   = false;
  bool print_serial : 1 = false;

  // sorting options
  bool sort_reverse : 1 = false;
  sort_key sort_key     = sort_key::name;
  time_src timestamp    = time_src::last_modified;

  // size options
  size_t size_block = 0;
};

option_data parse_options(const int argc, const char** argv) {
  using mtap::option, mtap::pos_arg;
  option_data data;
  mtap::parser opts(
    option<"--help", 0>([&] {
      usage(argv[0]);
      exit(0);
    }),
    option<"-A", 0>(
      [&] { data.contents = option_data::list_values::list_hidden; }),
    option<"-a", 0>(
      [&] { data.contents = option_data::list_values::list_all; }),
    option<"-C", 0>([&] { data.format = option_data::format::columns_v; }),
    option<"-c", 0>(
      [&] { data.timestamp = option_data::time_src::last_stat_change; }),
    option<"-d", 0>([&] { data.list_dir_contents = false; }),
    option<"-F", 0>([&] { data.indicators = option_data::indicators::all; }),
    option<"-f", 0>([&] {
      data.sort_key = option_data::sort_key::none;
      data.contents = option_data::list_values::list_all;
    }),
    option<"-g", 0>([&] {
      data.print_user = false;
      data.format     = option_data::format::details;
    }),
    option<"-H", 0>(
      [&] { data.link_bhv = option_data::resolve_links::specified; }),
    option<"-i", 0>([&] { data.print_serial = true; }),
    option<"-k", 0>([&] { data.size_block = 10; }),
    option<"-L", 0>([&] { data.size_block = 10; }),
    option<"-l", 0>([&] { data.format = option_data::format::details; }),
    option<"-m", 0>([&] { data.format = option_data::format::csv; }),
    option<"-n", 0>([&] { data.print_ids = true; }),
    option<"-o", 0>([&] { data.print_group = false; }),
    option<"-p", 0>([&] { data.indicators = option_data::indicators::slash; }),
    option<"-q", 0>([&] { data.esc_chars = option_data::escape_chars::qmark; }),
    option<"-R", 0>([&] { data.recursive = true; }),
    option<"-r", 0>([&] { data.sort_reverse = true; }),
    option<"-S", 0>([&] { data.sort_key = option_data::sort_key::size; }),
    option<"-s", 0>([&] { data.size_block = 20; }),
    option<"-t", 0>([&] { data.sort_key = option_data::sort_key::time; }),
    option<"-u", 0>(
      [&] { data.timestamp = option_data::time_src::last_accessed; }),
    option<"-x", 0>([&] { data.format = option_data::format::columns_h; }),
    option<"-1", 0>([&] { data.format = option_data::format::lines; }),
    pos_arg([&](std::string_view arg) { data.paths.push_back(arg); }));
  opts.parse(argc, argv);
  return data;
}

int main(const int argc, const char* argv[]) {
  using namespace std::string_view_literals;

  auto config = parse_options(argc, argv);
}