#include <bitset>
#include <iostream>
#include <string_view>
#include <vector>

#include <fmt/core.h>

using namespace std::literals::string_view_literals;

using args_t = std::vector<std::string_view>;

void usage(std::string_view argv0) {
  std::cout << fmt::format(R"msg(
usage: {0} [-ne]... [MESSAGE]...
   or: {0} [--help]
Prints the MESSAGEs to standard output.

Options:
  -n        do not print a trailing newline
  -e        interpret backslash escapes

  --help    print this help page and exit

Supported escape sequences are:
  \\        backslash
  \a        alert (BEL)
  \b        backspace
  \e        escape
  \f        form feed
  \n        newline (LF)
  \r        carriage return (CR)
  \t        tab
  \v        vertical tab
  \0        null character (NUL)
  \0NNN     octal escape for value NNN (1-3 digits)
  \xNN      hexadecimal escape for value NN (1-2 digits)

NOTE: Some shells have echo as a builtin command, which will likely override 
this one. Please check your shell's manual for information on its version.
)msg"sv.substr(1), argv0);
}

[[gnu::always_inline]] inline bool is_octal_digit(char c) {
  return (c >= '0' && c <= '7');
}

[[gnu::always_inline]] inline bool is_hex_digit(char c) {
  return (c >= '0' && c <= '9') | (c >= 'A' && c <= 'F') | (c >= 'a' && c <= 'f');
}

[[gnu::always_inline]] inline char extract_hex_digit(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  else if (c >= 'A' && c <= 'f') return c - 'A' + 10;
  else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return '\0';
}



std::string process_escapes(const std::string& in) {
  std::string out;
  out.reserve(in.size());
  
  enum class state {
    normal,
    begin_esc,
    oct_esc,
    hex_esc
  } status = state::normal;
  uint8_t remaining;
  uint8_t val;
  
  for (auto i = in.begin(); i < in.end(); ++i) {
    switch (status) {
      case state::normal: {
        if (*i == '\\')
          status = state::begin_esc;
        else
          out.push_back(*i);
      } break;
      case state::begin_esc: {
        switch (*i) {
          case '0': {
            // check if next character is an octal digit (if it exists)
            if (i + 1 < in.end() && is_octal_digit(i[1])) {
              status = state::oct_esc;
              remaining = 3;
              continue;
            }
            out.push_back('\0');
            status = state::normal;
            continue;
          }
          case 'x': {
            // check if next character is a hex digit (if it exists)
            if (i + 1 < in.end() && is_hex_digit(i[1])) {
              status = state::hex_esc;
              remaining = 2;
              continue;
            }
            out.append({'\\', 'x'});
            status = state::normal;
            continue;
          }
          case '\\': {
            out.push_back('\\');
            status = state::normal;
          } break;
          case 'a': {
            out.push_back('\a');
            status = state::normal;
          } break;
          case 'b': {
            out.push_back('\b');
            status = state::normal;
          } break;
          case 'e': {
            out.push_back('\e');
            status = state::normal;
          } break;
          case 'f': {
            out.push_back('\f');
            status = state::normal;
          } break;
          case 'n': {
            out.push_back('\n');
            status = state::normal;
          } break;
          case 'r': {
            out.push_back('\r');
            status = state::normal;
          } break;
          case 't': {
            out.push_back('\t');
            status = state::normal;
          } break;
          case 'v': {
            out.push_back('\v');
            status = state::normal;
          } break;
          default: {
            out.append({'\\', *i});
          } break;
        }
      } break;
      case state::oct_esc: {
        // add a new digit
        val <<= 3;
        val |= (*i - '0');
        --remaining;
        
        if (remaining > 0 && i + 1 < in.end() && is_octal_digit(i[1])) break;
        
        out.push_back(char(val));
        // reset state
        remaining = 0;
        val = '\0';
        status = state::normal;
      } break;
      case state::hex_esc: {
        val <<= 4;
        val |= extract_hex_digit(*i);
        --remaining;
        
        if (i + 1 < in.end() && is_hex_digit(i[1])) break;
        
        out.push_back(char(val));
        // reset state
        remaining = 0;
        val = '\0';
        status = state::normal;
      } break;
    }
  }
  return out;
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    std::cout << "\n";
    return 0;
  }
  args_t args(argv, argv + argc);
  
  // check for help option
  if (args[1] == "--help") {
    usage(args[0]);
    return 0;
  }
  
  // process CLI arguments manually, because this command is special
  bool allow_opts = true;
  struct echo_opts {
    bool no_nl;
    bool escapes;
  } opts {false, false};
  std::string out;
  for (auto i = ++args.begin(); i != args.end(); ++i) {
    if (allow_opts) {
      if ((*i)[0] == '-') {
        auto j = i->begin(); ++j;
        echo_opts new_opts = opts;
        for (; j != i->end(); ++j) {
          switch (*j) {
            case 'e':
              new_opts.escapes = true;
              break;
            case 'n':
              new_opts.no_nl = true;
              break;
            default:
              goto echo_past_save_opts;
          }
        }
        opts = new_opts;
        continue;
      }
      else {
        echo_past_save_opts:
        allow_opts = false;
        out = *i;
      }
    }
    else {
      bool add_space = !out.empty();
      out.reserve(out.size() + i->size() + add_space);
      if (add_space) out.push_back(' ');
      out.append(*i);
    }
  }
  if (opts.escapes) out = process_escapes(out);
  std::cout << out;
  if (!opts.no_nl) std::cout << '\n';
  std::cout.flush();
  
  return 0;
}