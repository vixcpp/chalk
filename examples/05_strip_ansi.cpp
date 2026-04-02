/**
 * @file 05_strip_ansi.cpp
 * @brief ANSI stripping example
 */

#include <iostream>
#include <string>
#include <vix/chalk/chalk.hpp>

int main()
{
  using namespace vix::chalk;

  const std::string styled = red().bold()("failure");
  const std::string plain = strip_ansi(styled);

  std::cout << "Styled: " << styled << '\n';
  std::cout << "Plain:  " << plain << '\n';

  return 0;
}
