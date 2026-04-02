/**
 * @file 02_chaining.cpp
 * @brief Chaining style examples
 */

#include <iostream>
#include <vix/chalk/chalk.hpp>

int main()
{
  using namespace vix::chalk;

  std::cout << red().bold()("fatal error") << '\n';
  std::cout << green().underline()("operation completed") << '\n';
  std::cout << yellow().italic()("careful with this step") << '\n';
  std::cout << blue().bold().underline()("important message") << '\n';
  std::cout << magenta().strikethrough()("deprecated") << '\n';
  std::cout << cyan().dim()("subtle details") << '\n';

  return 0;
}
