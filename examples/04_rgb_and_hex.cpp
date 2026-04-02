/**
 * @file 04_rgb_and_hex.cpp
 * @brief RGB and HEX color examples
 */

#include <iostream>
#include <vix/chalk/chalk.hpp>

int main()
{
  using namespace vix::chalk;

  std::cout << rgb(255, 153, 0)("brand orange") << '\n';
  std::cout << rgb(0, 200, 120).bold()("custom success") << '\n';
  std::cout << bgRgb(20, 20, 20).white()("dark block") << '\n';

  std::cout << hex("#ff9900")("hex orange") << '\n';
  std::cout << hex("#22c55e").bold()("hex green") << '\n';
  std::cout << bgHex("#1f2937").white().bold()("panel title") << '\n';

  return 0;
}
