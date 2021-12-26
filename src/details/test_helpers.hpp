#ifndef _CXCU_DETAILS_TEST_HELPERS_HPP_
#define _CXCU_DETAILS_TEST_HELPERS_HPP_

#include <cstdint>
#include <stdexcept>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <unordered_map>

namespace coreutils::test {
  enum class tag {
    unary_condition,
    binary_condition,
    logical,
    special_op
  };

  enum class opcode : uint16_t {
    file_block_special,
    file_char_special,
    file_directory,
    file_exists,
    file_regular_file,
    file_set_group_id,
    file_symbolic_link,
    str_not_empty,
    file_fifo,
    file_readable,
    file_socket,
    file_not_empty,
    fd_open,
    file_set_user_id,
    file_writable,
    file_executable,
    str_empty,
    str_equal,
    str_not_equal,
    num_equal,
    num_not_equal,
    num_greater,
    num_greater_equal,
    num_less,
    num_less_equal,
    bool_and,
    bool_or,
    bool_not,
    paren_left,
    paren_right
  };

  inline tag opcode_tag(opcode op) {
    uint16_t val = static_cast<uint16_t>(op);
    if (val <= static_cast<uint16_t>(opcode::str_empty))
      return tag::unary_condition;
    if (val <= static_cast<uint16_t>(opcode::num_less_equal))
      return tag::binary_condition;
    if (val <= static_cast<uint16_t>(opcode::paren_right))
      return tag::logical;
    throw std::logic_error("INTERNAL ERROR: test tag received invalid opcode");
  }

  using token_t = std::variant<std::string, opcode>;

  std::optional<opcode> try_parse_opcode(const std::string& str);

  std::optional<bool> try_test_unary(opcode op, const std::string& p1);
  
  std::optional<bool> try_test_binary(
    opcode op, const std::string& p1, const std::string& p2);
  
  std::string eval_conditions(const std::vector<std::string>& args);
  bool eval_logic(const std::string& str);
}  // namespace coreutils::test
#endif