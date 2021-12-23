#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

int main(int argc, char* c_argv[]) {
  if (argc == 1) {
    std::cout << "\n";
    return 0;
  }
  std::vector<std::string_view> argv(c_argv, c_argv + argc);
  
  auto it = argv.begin();
  std::cout << *it++;
  while (it != argv.end()) std::cout << ", " << *it++;
  std::cout << "\n";
}