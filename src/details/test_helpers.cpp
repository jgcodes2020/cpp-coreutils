#include "test_helpers.hpp"
#include <cctype>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "parse_error.hpp"

#include <fcntl.h>
#include <fmt/core.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std::literals::string_literals;
namespace fs = std::filesystem;

#ifdef __GNUC__
typedef unsigned __int128 uint128_t;
typedef signed __int128 int128_t;
#endif

namespace {
#if defined(_WIN32)
  std::pair<int32_t, int32_t> imul_split(int32_t x, int32_t y) {
    int64_t res = int64_t(x) * int64_t(y);
    return {res, res >> 32};
  }
#elif defined(__linux__)
  std::pair<int64_t, int64_t> imul_split(int64_t x, int64_t y) {
    int128_t res = int128_t(x) * int128_t(y);
    return {res, res >> 64};
  }
#endif

  long parse_int(const std::string& str) {
    auto it = str.begin();
    while (std::isspace(*it, std::locale()))
      ++it;

    long result;
    while (!std::isspace(*it, std::locale())) {
      if (!std::isdigit(*it, std::locale())) {
        throw std::invalid_argument(
          fmt::format("\"{}\" is not an integer", str));
      }
      auto [lo, hi] = imul_split(result, 10);
      if (hi != 0 && hi != -1) {
        throw std::out_of_range(
          fmt::format("\"{}\" is too large to be used", str));
      }
      result = lo;
      result += (*it++ - '0');
    }

    while (it != str.end()) {
      if (!std::isspace(*it, std::locale())) {
        throw std::invalid_argument(
          fmt::format("\"{}\" contains more than just one integer", str));
      }
    }
    return result;
  }
}  // namespace

namespace coreutils::test {
  std::optional<opcode> try_parse_opcode(const std::string& str) {
    static const std::unordered_map<std::string, opcode> map = {
      {"-b"s, opcode::file_block_special},
      {"-c"s, opcode::file_char_special},
      {"-d"s, opcode::file_directory},
      {"-e"s, opcode::file_exists},
      {"-f"s, opcode::file_regular_file},
      {"-g"s, opcode::file_set_group_id},
      {"-h"s, opcode::file_symbolic_link},
      {"-L"s, opcode::file_symbolic_link},
      {"-n"s, opcode::str_not_empty},
      {"-p"s, opcode::file_fifo},
      {"-r"s, opcode::file_readable},
      {"-u"s, opcode::file_set_user_id},
      {"-w"s, opcode::file_writable},
      {"-x"s, opcode::file_executable},
      {"-z"s, opcode::str_empty},
      {"="s, opcode::str_equal},
      {"!="s, opcode::str_not_equal},
      {"-eq"s, opcode::num_equal},
      {"-ne"s, opcode::num_not_equal},
      {"-gt"s, opcode::num_greater},
      {"-ge"s, opcode::num_greater_equal},
      {"-lt"s, opcode::num_less_equal},
      {"-a"s, opcode::bool_and},
      {"-o"s, opcode::bool_or},
      {"!"s, opcode::bool_not},
      {"("s, opcode::paren_left},
      {")"s, opcode::paren_right},
    };

    auto res = map.find(str);
    if (res != map.end())
      return res->second;
    else
      return std::nullopt;
  }
  std::optional<bool> try_test_unary(opcode op, const std::string& p1) {
    static const std::unordered_map<opcode, bool (*)(const std::string&)> map {
      {
        opcode::file_block_special,
        [](const std::string& path) -> bool { return fs::is_block_file(path); },
      },
      {
        opcode::file_char_special,
        [](const std::string& path) -> bool {
          return fs::is_character_file(path);
        },
      },
      {
        opcode::file_directory,
        [](const std::string& path) -> bool { return fs::is_directory(path); },
      },
      {
        opcode::file_exists,
        [](const std::string& path) -> bool { return fs::exists(path); },
      },
      {
        opcode::file_regular_file,
        [](const std::string& path) -> bool {
          return fs::is_regular_file(path);
        },
      },
      {
        opcode::file_set_group_id,
        [](const std::string& path) -> bool {
          auto permissions = fs::status(path).permissions();
          return (permissions & fs::perms::set_gid) != fs::perms::none;
        },
      },
      {
        opcode::file_symbolic_link,
        [](const std::string& path) -> bool { return fs::is_symlink(path); },
      },
      {
        opcode::str_not_empty,
        [](const std::string& str) -> bool { return !str.empty(); },
      },
      {
        opcode::file_fifo,
        [](const std::string& str) -> bool { return fs::is_fifo(str); },
      },
      {
        opcode::fd_open,
        [](const std::string& str) -> bool {
          auto fd = parse_int(str);
          return fcntl(fd, F_GETFD) != -1;
        },
      },
      {
        opcode::file_readable,
        [](const std::string& path) -> bool {
          return euidaccess(std::string(path).c_str(), R_OK) == 0;
        },
      },
      {
        opcode::file_set_user_id,
        [](const std::string& path) -> bool {
          auto permissions = fs::status(path).permissions();
          return (permissions & fs::perms::set_uid) != fs::perms::none;
        },
      },
      {
        opcode::file_writable,
        [](const std::string& path) -> bool {
          return euidaccess(std::string(path).c_str(), W_OK) == 0;
        },
      },
      {
        opcode::file_executable,
        [](const std::string& path) -> bool {
          return euidaccess(std::string(path).c_str(), X_OK) == 0;
        },
      },
      {
        opcode::str_empty,
        [](const std::string& str) -> bool { return str.empty(); },
      },
    };

    auto res = map.find(op);
    if (res != map.end())
      return res->second(p1);
    else
      return std::nullopt;
  }
  std::optional<bool> try_test_binary(
    opcode op, const std::string& p1, const std::string& p2) {
    static const std::unordered_map<
      opcode, bool (*)(const std::string&, const std::string&)>
      map {
        {
          opcode::str_equal,
          [](const std::string& a, const std::string& b) -> bool {
            return a == b;
          },
        },
        {
          opcode::str_not_equal,
          [](const std::string& a, const std::string& b) -> bool {
            return a == b;
          },
        },
        {
          opcode::num_equal,
          [](const std::string& a, const std::string& b) -> bool {
            signed long x, y;
            try {
              x = parse_int(a);
              y = parse_int(b);
            }
            catch (const std::exception&) {
              std::throw_with_nested(coreutils::parse_error(
                "Operator -eq expects integer operands"));
            }
            return x == y;
          },
        },
        {
          opcode::num_not_equal,
          [](const std::string& a, const std::string& b) -> bool {
            signed long x, y;
            try {
              x = parse_int(a);
              y = parse_int(b);
            }
            catch (const std::exception&) {
              std::throw_with_nested(coreutils::parse_error(
                "Operator -ne expects integer operands"));
            }
            return x != y;
          },
        },
        {
          opcode::num_greater,
          [](const std::string& a, const std::string& b) -> bool {
            signed long x, y;
            try {
              x = parse_int(a);
              y = parse_int(b);
            }
            catch (const std::exception&) {
              std::throw_with_nested(coreutils::parse_error(
                "Operator -gt expects integer operands"));
            }
            return x > y;
          },
        },
        {
          opcode::num_greater_equal,
          [](const std::string& a, const std::string& b) -> bool {
            signed long x, y;
            try {
              x = parse_int(a);
              y = parse_int(b);
            }
            catch (const std::exception&) {
              std::throw_with_nested(coreutils::parse_error(
                "Operator -ge expects integer operands"));
            }
            return x >= y;
          },
        },
        {
          opcode::num_less,
          [](const std::string& a, const std::string& b) -> bool {
            signed long x, y;
            try {
              x = parse_int(a);
              y = parse_int(b);
            }
            catch (const std::exception&) {
              std::throw_with_nested(coreutils::parse_error(
                "Operator -lt expects integer operands"));
            }
            return x > y;
          },
        },
        {
          opcode::num_less_equal,
          [](const std::string& a, const std::string& b) -> bool {
            signed long x, y;
            try {
              x = parse_int(a);
              y = parse_int(b);
            }
            catch (const std::exception&) {
              std::throw_with_nested(coreutils::parse_error(
                "Operator -le expects integer operands"));
            }
            return x >= y;
          },
        },
      };

    auto res = map.find(op);
    if (res != map.end())
      return res->second(p1, p2);
    else
      return std::nullopt;
  }

  std::string eval_conditions(const std::vector<std::string>& args) {
    std::string out;
    for (auto i0 = args.begin(); i0 < args.end();) {
      auto op0         = try_parse_opcode(*i0);
      bool op0_present = op0.has_value();

      // handle parentheses and NOT
      if (op0_present) {
        switch (*op0) {
        case opcode::bool_not: {
          out.push_back('!');
          i0 += 1;
          continue;
        }
        case opcode::paren_left: {
          out.push_back('(');
          i0 += 1;
          continue;
        }
        case opcode::paren_right: {
          out.push_back(')');
          i0 += 1;
          continue;
        }
        case opcode::bool_and: {
          if (i0 > args.begin()) {
            out.push_back('&');
            i0 += 1;
            continue;
          }
        } break;
        case opcode::bool_or: {
          if (i0 > args.begin()) {
            out.push_back('|');
            i0 += 1;
            continue;
          }
        } break;
        default:
          break;
        }
      }
      // End of arguments: implicit non-null check
      auto i1 = i0 + 1;
      if (i1 == args.end()) {
        out.push_back('0' + int(!i0->empty()));
        break;
      }

      // check for binary condition
      auto i2 = i0 + 2;
      if (i2 < args.end()) {
        auto op1         = try_parse_opcode(*i1);
        bool op1_present = op1.has_value();
        if (op1_present) {
          if (opcode_tag(*op1) == tag::binary_condition) {
            bool res = try_test_binary(*op1, *i0, *i2).value();
            out.push_back('0' + int(res));
            i0 += 3;
            continue;
          }
          else if (*op1 == opcode::bool_and || *op1 == opcode::bool_or) {
            out.push_back('0' + int(!i0->empty()));
            out.push_back((*op1 == opcode::bool_and) ? '&' : '|');
            i0 += 2;
            continue;
          }
        }
      }

      // check for unary condition
      if (opcode_tag(*op0) == tag::unary_condition) {
        bool res = try_test_unary(*op0, *i1).value();
        out.push_back('0' + int(res));
        i0 += 2;
        continue;
      }
    }
    return out;
  }

  bool parse_ops(std::string::const_iterator& expr, size_t minp, const std::string& str);
  bool parse_unit(std::string::const_iterator& expr, const std::string& str) {
    switch (*expr) {
    case '0':
      ++expr;
      return false;
    case '1':
      ++expr;
      return true;
    case '!':
      ++expr;
      return !parse_unit(expr, str);
    case '(':
      ++expr;
      return parse_ops(expr, 0, str);
    }
    throw coreutils::parse_error(fmt::format("Unexpected char {}", *expr));
  }

  bool parse_ops(std::string::const_iterator& expr, size_t minp, const std::string& str) {
    static const std::unordered_map<char, size_t> bool_ops {
      {'&', 2},
      {'|', 1},
    };

    bool result = parse_unit(expr, str);
    decltype(bool_ops.find('E')) it;
    while (expr < str.end() && (it = bool_ops.find(*expr)) != bool_ops.end() &&
           it->second >= minp) {
      size_t newp = it->second + 1;
      char op     = *expr;
      ++expr;
      bool next = parse_ops(expr, newp, str);
      result    = (op == '|') ?
           result || next :
           (op == '&') ?
           result && next :
           throw coreutils::parse_error("INTERNAL ERROR: bad binary op");
    }
    return result;
    
  }
  
  bool eval_logic(const std::string& str) {
    if (regex_search(str, std::regex("[01]{2}")))
      throw coreutils::parse_error("Two conditions are adjacent");
    
    std::string::const_iterator x = str.cbegin();
    
    return parse_ops(x, 1, str);
  }
}  // namespace coreutils::test