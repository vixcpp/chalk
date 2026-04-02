/**
 * @file 03_backgrounds.cpp
 * @brief Background color examples
 */

#include <iostream>
#include <vix/chalk/chalk.hpp>

int main()
{
  using namespace vix::chalk;

  std::cout << bgRed().white().bold()(" ERROR ") << '\n';
  std::cout << bgGreen().black().bold()(" OK ") << '\n';
  std::cout << bgYellow().black()(" WARNING ") << '\n';
  std::cout << bgBlue().white()(" INFO ") << '\n';
  std::cout << bgMagenta().white()(" TAG ") << '\n';
  std::cout << bgCyan().black()(" NOTE ") << '\n';

  return 0;
}
