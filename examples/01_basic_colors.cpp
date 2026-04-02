/**
 * @file 01_basic_colors.cpp
 * @brief Basic foreground color examples
 */

#include <iostream>
#include <vix/chalk/chalk.hpp>

int main()
{
  using namespace vix::chalk;

  std::cout << red("error") << '\n';
  std::cout << green("success") << '\n';
  std::cout << yellow("warning") << '\n';
  std::cout << blue("info") << '\n';
  std::cout << magenta("highlight") << '\n';
  std::cout << cyan("note") << '\n';
  std::cout << white("plain white") << '\n';
  std::cout << gray("secondary") << '\n';

  return 0;
}
