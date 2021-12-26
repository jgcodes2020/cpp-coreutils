#ifndef _CXCU_DETAILS_PARSE_ERROR_HPP_
#define _CXCU_DETAILS_PARSE_ERROR_HPP_
#include <stdexcept>
namespace coreutils {
  // Represents an error that occurs during parsing
  class parse_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
  };
}
#endif