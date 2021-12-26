#ifndef _CXCU_DETAILS_DEMANGLE_HPP_
#define _CXCU_DETAILS_DEMANGLE_HPP_
#include <cstdlib>
#include <memory>
#include <string>
#include <typeinfo>

#include <cxxabi.h>

namespace coreutils {
#ifdef __GNUC__
  inline std::string pretty_name(const std::type_info& type) {
    int status = -4;

    std::unique_ptr<char, void (&)(void*)> ptr {
      abi::__cxa_demangle(type.name(), nullptr, nullptr, &status), std::free};
    return (status == 0) ? ptr.get() : type.name();
  }
#else
  inline std::string pretty_name(const std::type_info& type) {
    return type.name();
  }
#endif
}  // namespace coreutils
#endif