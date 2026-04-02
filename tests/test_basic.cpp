/**
 * @file test_basic.cpp
 * @brief Basic tests for vix::chalk
 *
 * @version 0.1.0
 * @author Gaspard Kirira
 * @copyright (c) 2026
 * @license MIT
 */

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include <vix/chalk/chalk.hpp>

namespace
{
  void expect_true(bool condition, const std::string &message)
  {
    if (!condition)
    {
      throw std::runtime_error(message);
    }
  }

  void expect_equal(const std::string &actual,
                    const std::string &expected,
                    const std::string &message)
  {
    if (actual != expected)
    {
      throw std::runtime_error(
          message + "\nExpected: [" + expected + "]\nActual:   [" + actual + "]");
    }
  }

  void test_plain_text_when_disabled()
  {
    vix::chalk::setEnabled(false);

    expect_equal(vix::chalk::red("error"), "error",
                 "red(text) should return plain text when styling is disabled");

    expect_equal(vix::chalk::red().bold()("fatal"), "fatal",
                 "chained style should return plain text when styling is disabled");
  }

  void test_basic_foreground_color()
  {
    vix::chalk::setEnabled(true);

    expect_equal(vix::chalk::red("error"), "\033[31merror\033[0m",
                 "red(text) should wrap text with the correct ANSI sequence");
  }

  void test_chained_styles()
  {
    vix::chalk::setEnabled(true);

    expect_equal(vix::chalk::red().bold()("fatal"), "\033[31;1mfatal\033[0m",
                 "red().bold()(text) should apply chained ANSI codes in order");
  }

  void test_background_and_foreground()
  {
    vix::chalk::setEnabled(true);

    expect_equal(vix::chalk::bgBlue().white()("info"), "\033[44;37minfo\033[0m",
                 "bgBlue().white()(text) should apply background and foreground colors");
  }

  void test_rgb_style()
  {
    vix::chalk::setEnabled(true);

    expect_equal(vix::chalk::rgb(255, 153, 0)("brand"),
                 "\033[38;2;255;153;0mbrand\033[0m",
                 "rgb(r,g,b)(text) should generate the correct ANSI truecolor sequence");
  }

  void test_hex_style()
  {
    vix::chalk::setEnabled(true);

    expect_equal(vix::chalk::hex("#ff9900")("brand"),
                 "\033[38;2;255;153;0mbrand\033[0m",
                 "hex(#rrggbb)(text) should generate the correct ANSI truecolor sequence");
  }

  void test_strip_ansi()
  {
    vix::chalk::setEnabled(true);

    const std::string styled = vix::chalk::green().bold()("success");
    expect_equal(vix::chalk::strip_ansi(styled), "success",
                 "strip_ansi should remove ANSI escape sequences");
  }

  void test_invalid_hex_throws()
  {
    vix::chalk::setEnabled(true);

    bool thrown = false;

    try
    {
      (void)vix::chalk::hex("#zzzzzz")("broken");
    }
    catch (const std::invalid_argument &)
    {
      thrown = true;
    }

    expect_true(thrown, "hex with invalid digits should throw std::invalid_argument");
  }

  void run_all_tests()
  {
    test_plain_text_when_disabled();
    test_basic_foreground_color();
    test_chained_styles();
    test_background_and_foreground();
    test_rgb_style();
    test_hex_style();
    test_strip_ansi();
    test_invalid_hex_throws();
  }
} // namespace

int main()
{
  try
  {
    run_all_tests();
    std::cout << "All chalk tests passed.\n";
    return EXIT_SUCCESS;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Test failure: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
