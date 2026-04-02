/**
 * @file chalk.hpp
 * @brief Terminal string styling utilities for C++
 *
 * @version 0.1.0
 * @author Gaspard Kirira
 * @copyright (c) 2026 Gaspard Kirira
 * @license MIT
 *
 * @details
 * This header provides a small and expressive API for styling terminal text
 * using ANSI escape sequences.
 *
 * Features:
 * - chainable styles
 * - foreground colors
 * - background colors
 * - bright colors
 * - RGB colors
 * - HEX colors
 * - ANSI stripping utility
 * - terminal color support detection
 *
 * Example:
 * @code
 * #include <iostream>
 * #include <vix/chalk/chalk.hpp>
 *
 * int main()
 * {
 *   std::cout << vix::chalk::green("success") << '\n';
 *   std::cout << vix::chalk::red().bold()("error") << '\n';
 *   std::cout << vix::chalk::bgBlue().white().bold()("info") << '\n';
 *   std::cout << vix::chalk::hex("#ff9900").underline()("highlight") << '\n';
 * }
 * @endcode
 */

#ifndef VIX_CHALK_CHALK_HPP
#define VIX_CHALK_CHALK_HPP

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <initializer_list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace vix::chalk
{
  /**
   * @brief Utility helpers used internally by the styling system.
   */
  namespace detail
  {
    inline int clamp_channel(int value)
    {
      return std::max(0, std::min(255, value));
    }

    inline bool is_hex_digit(char c)
    {
      return std::isxdigit(static_cast<unsigned char>(c)) != 0;
    }

    inline int hex_value(char c)
    {
      if (c >= '0' && c <= '9')
      {
        return c - '0';
      }
      if (c >= 'a' && c <= 'f')
      {
        return 10 + (c - 'a');
      }
      if (c >= 'A' && c <= 'F')
      {
        return 10 + (c - 'A');
      }
      return 0;
    }

    inline std::string join_codes(const std::vector<std::string> &codes)
    {
      std::string result;
      for (std::size_t i = 0; i < codes.size(); ++i)
      {
        if (i > 0)
        {
          result += ';';
        }
        result += codes[i];
      }
      return result;
    }

    inline bool starts_with_escape(std::string_view text, std::size_t pos)
    {
      return pos + 1 < text.size() && text[pos] == '\033' && text[pos + 1] == '[';
    }

    inline bool is_ansi_terminator(char c)
    {
      return c >= '@' && c <= '~';
    }

    inline std::string to_lower_copy(std::string value)
    {
      std::transform(value.begin(), value.end(), value.begin(),
                     [](unsigned char ch)
                     { return static_cast<char>(std::tolower(ch)); });
      return value;
    }

    inline std::string normalize_hex(std::string hex)
    {
      if (!hex.empty() && hex.front() == '#')
      {
        hex.erase(hex.begin());
      }

      if (hex.size() == 3)
      {
        std::string expanded;
        expanded.reserve(6);
        for (char c : hex)
        {
          if (!is_hex_digit(c))
          {
            throw std::invalid_argument("invalid hex color");
          }
          expanded.push_back(c);
          expanded.push_back(c);
        }
        return expanded;
      }

      if (hex.size() != 6)
      {
        throw std::invalid_argument("hex color must have 3 or 6 hex digits");
      }

      for (char c : hex)
      {
        if (!is_hex_digit(c))
        {
          throw std::invalid_argument("invalid hex color");
        }
      }

      return hex;
    }

    struct Rgb
    {
      int r;
      int g;
      int b;
    };

    inline Rgb parse_hex_color(const std::string &hex)
    {
      const std::string normalized = normalize_hex(hex);

      const int r = hex_value(normalized[0]) * 16 + hex_value(normalized[1]);
      const int g = hex_value(normalized[2]) * 16 + hex_value(normalized[3]);
      const int b = hex_value(normalized[4]) * 16 + hex_value(normalized[5]);

      return {r, g, b};
    }

    inline const char *get_env(const char *name)
    {
      return std::getenv(name);
    }

    inline bool env_is_set(const char *name)
    {
      const char *value = get_env(name);
      return value != nullptr && *value != '\0';
    }

    inline bool env_equals(const char *name, const char *expected)
    {
      const char *value = get_env(name);
      if (value == nullptr)
      {
        return false;
      }
      return std::string_view(value) == std::string_view(expected);
    }

    inline bool probably_supports_color()
    {
      if (env_is_set("NO_COLOR"))
      {
        return false;
      }

      if (env_is_set("FORCE_COLOR"))
      {
        return true;
      }

      if (env_equals("TERM", "dumb"))
      {
        return false;
      }

      if (env_is_set("COLORTERM"))
      {
        return true;
      }

      const char *term = get_env("TERM");
      if (term == nullptr)
      {
        return false;
      }

      const std::string lower = to_lower_copy(term);

      return lower.find("xterm") != std::string::npos ||
             lower.find("color") != std::string::npos ||
             lower.find("ansi") != std::string::npos ||
             lower.find("screen") != std::string::npos ||
             lower.find("tmux") != std::string::npos ||
             lower.find("rxvt") != std::string::npos ||
             lower.find("linux") != std::string::npos ||
             lower.find("cygwin") != std::string::npos;
    }

    inline std::string make_ansi_prefix(const std::vector<std::string> &codes)
    {
      return "\033[" + join_codes(codes) + "m";
    }

    inline std::string make_ansi_reset()
    {
      return "\033[0m";
    }

    inline std::string code_8bit_rgb_prefix(bool background, int r, int g, int b)
    {
      std::ostringstream oss;
      oss << (background ? "48" : "38") << ";2;"
          << clamp_channel(r) << ';'
          << clamp_channel(g) << ';'
          << clamp_channel(b);
      return oss.str();
    }
  } // namespace detail

  /**
   * @brief Global color configuration.
   */
  class Config
  {
  public:
    /**
     * @brief Returns whether styling is enabled globally.
     *
     * @return true if styles are enabled
     * @return false if styles are disabled
     */
    static bool enabled()
    {
      return enabled_flag();
    }

    /**
     * @brief Enables or disables styling globally.
     *
     * @param value New global state.
     */
    static void setEnabled(bool value)
    {
      enabled_flag() = value;
    }

  private:
    static bool &enabled_flag()
    {
      static bool value = detail::probably_supports_color();
      return value;
    }
  };

  /**
   * @brief Returns whether ANSI styling is globally enabled.
   *
   * @return true if enabled
   * @return false if disabled
   */
  inline bool enabled()
  {
    return Config::enabled();
  }

  /**
   * @brief Enables or disables ANSI styling globally.
   *
   * @param value New global state.
   */
  inline void setEnabled(bool value)
  {
    Config::setEnabled(value);
  }

  /**
   * @brief Returns whether the current terminal environment likely supports colors.
   *
   * @return true if color support is likely available
   * @return false otherwise
   */
  inline bool supports_color()
  {
    return detail::probably_supports_color();
  }

  /**
   * @brief Removes ANSI escape sequences from a string.
   *
   * @param input Styled text.
   * @return std::string Plain text without ANSI sequences.
   */
  inline std::string strip_ansi(std::string_view input)
  {
    std::string output;
    output.reserve(input.size());

    std::size_t i = 0;
    while (i < input.size())
    {
      if (detail::starts_with_escape(input, i))
      {
        i += 2;
        while (i < input.size() && !detail::is_ansi_terminator(input[i]))
        {
          ++i;
        }
        if (i < input.size())
        {
          ++i;
        }
        continue;
      }

      output.push_back(input[i]);
      ++i;
    }

    return output;
  }

  /**
   * @brief Represents a chainable text style.
   *
   * @details
   * A Style instance stores ANSI formatting codes and applies them when called
   * with a text value.
   */
  class Style
  {
  public:
    /**
     * @brief Creates an empty style.
     */
    Style() = default;

    /**
     * @brief Creates a style from a list of ANSI codes.
     *
     * @param codes ANSI numeric codes.
     */
    Style(std::initializer_list<std::string> codes) : codes_(codes) {}

    /**
     * @brief Applies the style to the given text.
     *
     * @param text Input text.
     * @return std::string Styled text.
     */
    [[nodiscard]] std::string operator()(std::string_view text) const
    {
      return apply(text);
    }

    /**
     * @brief Applies the style to the given text.
     *
     * @param text Input text.
     * @return std::string Styled text.
     */
    [[nodiscard]] std::string apply(std::string_view text) const
    {
      if (!Config::enabled() || codes_.empty())
      {
        return std::string(text);
      }

      std::string result;
      const std::string prefix = detail::make_ansi_prefix(codes_);
      const std::string reset = detail::make_ansi_reset();

      result.reserve(prefix.size() + text.size() + reset.size());
      result += prefix;
      result += text;
      result += reset;
      return result;
    }

    /**
     * @brief Returns true if the style has no codes.
     *
     * @return true if empty
     * @return false otherwise
     */
    [[nodiscard]] bool empty() const noexcept
    {
      return codes_.empty();
    }

    /**
     * @brief Returns the ANSI codes currently stored in the style.
     *
     * @return const std::vector<std::string>& ANSI codes.
     */
    [[nodiscard]] const std::vector<std::string> &codes() const noexcept
    {
      return codes_;
    }

    /**
     * @brief Creates a new style with an additional ANSI code.
     *
     * @param code ANSI numeric code.
     * @return Style New extended style.
     */
    [[nodiscard]] Style with(std::string code) const
    {
      Style next = *this;
      next.codes_.push_back(std::move(code));
      return next;
    }

    /**
     * @brief Creates a new style with additional ANSI codes.
     *
     * @param extraCodes Additional ANSI numeric codes.
     * @return Style New extended style.
     */
    [[nodiscard]] Style with(std::initializer_list<std::string> extraCodes) const
    {
      Style next = *this;
      next.codes_.insert(next.codes_.end(), extraCodes.begin(), extraCodes.end());
      return next;
    }

    /**
     * @brief Clears all accumulated style codes.
     *
     * @return Style Empty style.
     */
    [[nodiscard]] Style clear() const
    {
      return Style{};
    }

    /**
     * @brief Sets bold text.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bold() const
    {
      return with("1");
    }

    /**
     * @brief Sets dim text.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style dim() const
    {
      return with("2");
    }

    /**
     * @brief Sets italic text.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style italic() const
    {
      return with("3");
    }

    /**
     * @brief Sets underlined text.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style underline() const
    {
      return with("4");
    }

    /**
     * @brief Sets blinking text.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style blink() const
    {
      return with("5");
    }

    /**
     * @brief Sets inverse text.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style inverse() const
    {
      return with("7");
    }

    /**
     * @brief Hides the text.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style hidden() const
    {
      return with("8");
    }

    /**
     * @brief Sets strikethrough text.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style strikethrough() const
    {
      return with("9");
    }

    /**
     * @brief Sets black foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style black() const
    {
      return with("30");
    }

    /**
     * @brief Sets red foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style red() const
    {
      return with("31");
    }

    /**
     * @brief Sets green foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style green() const
    {
      return with("32");
    }

    /**
     * @brief Sets yellow foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style yellow() const
    {
      return with("33");
    }

    /**
     * @brief Sets blue foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style blue() const
    {
      return with("34");
    }

    /**
     * @brief Sets magenta foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style magenta() const
    {
      return with("35");
    }

    /**
     * @brief Sets cyan foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style cyan() const
    {
      return with("36");
    }

    /**
     * @brief Sets white foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style white() const
    {
      return with("37");
    }

    /**
     * @brief Sets gray foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style gray() const
    {
      return with("90");
    }

    /**
     * @brief Sets grey foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style grey() const
    {
      return gray();
    }

    /**
     * @brief Sets bright red foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style redBright() const
    {
      return with("91");
    }

    /**
     * @brief Sets bright green foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style greenBright() const
    {
      return with("92");
    }

    /**
     * @brief Sets bright yellow foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style yellowBright() const
    {
      return with("93");
    }

    /**
     * @brief Sets bright blue foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style blueBright() const
    {
      return with("94");
    }

    /**
     * @brief Sets bright magenta foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style magentaBright() const
    {
      return with("95");
    }

    /**
     * @brief Sets bright cyan foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style cyanBright() const
    {
      return with("96");
    }

    /**
     * @brief Sets bright white foreground color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style whiteBright() const
    {
      return with("97");
    }

    /**
     * @brief Sets black background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgBlack() const
    {
      return with("40");
    }

    /**
     * @brief Sets red background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgRed() const
    {
      return with("41");
    }

    /**
     * @brief Sets green background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgGreen() const
    {
      return with("42");
    }

    /**
     * @brief Sets yellow background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgYellow() const
    {
      return with("43");
    }

    /**
     * @brief Sets blue background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgBlue() const
    {
      return with("44");
    }

    /**
     * @brief Sets magenta background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgMagenta() const
    {
      return with("45");
    }

    /**
     * @brief Sets cyan background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgCyan() const
    {
      return with("46");
    }

    /**
     * @brief Sets white background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgWhite() const
    {
      return with("47");
    }

    /**
     * @brief Sets gray background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgGray() const
    {
      return with("100");
    }

    /**
     * @brief Sets grey background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgGrey() const
    {
      return bgGray();
    }

    /**
     * @brief Sets bright red background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgRedBright() const
    {
      return with("101");
    }

    /**
     * @brief Sets bright green background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgGreenBright() const
    {
      return with("102");
    }

    /**
     * @brief Sets bright yellow background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgYellowBright() const
    {
      return with("103");
    }

    /**
     * @brief Sets bright blue background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgBlueBright() const
    {
      return with("104");
    }

    /**
     * @brief Sets bright magenta background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgMagentaBright() const
    {
      return with("105");
    }

    /**
     * @brief Sets bright cyan background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgCyanBright() const
    {
      return with("106");
    }

    /**
     * @brief Sets bright white background color.
     *
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgWhiteBright() const
    {
      return with("107");
    }

    /**
     * @brief Sets a foreground RGB color.
     *
     * @param r Red channel in range [0, 255].
     * @param g Green channel in range [0, 255].
     * @param b Blue channel in range [0, 255].
     * @return Style Updated style.
     */
    [[nodiscard]] Style rgb(int r, int g, int b) const
    {
      return with(detail::code_8bit_rgb_prefix(false, r, g, b));
    }

    /**
     * @brief Sets a background RGB color.
     *
     * @param r Red channel in range [0, 255].
     * @param g Green channel in range [0, 255].
     * @param b Blue channel in range [0, 255].
     * @return Style Updated style.
     */
    [[nodiscard]] Style bgRgb(int r, int g, int b) const
    {
      return with(detail::code_8bit_rgb_prefix(true, r, g, b));
    }

    /**
     * @brief Sets a foreground HEX color.
     *
     * @param value HEX value such as "#ff9900" or "f90".
     * @return Style Updated style.
     *
     * @throws std::invalid_argument if the HEX value is invalid.
     */
    [[nodiscard]] Style hex(const std::string &value) const
    {
      const detail::Rgb rgbValue = detail::parse_hex_color(value);
      return rgb(rgbValue.r, rgbValue.g, rgbValue.b);
    }

    /**
     * @brief Sets a background HEX color.
     *
     * @param value HEX value such as "#1e1e1e" or "333".
     * @return Style Updated style.
     *
     * @throws std::invalid_argument if the HEX value is invalid.
     */
    [[nodiscard]] Style bgHex(const std::string &value) const
    {
      const detail::Rgb rgbValue = detail::parse_hex_color(value);
      return bgRgb(rgbValue.r, rgbValue.g, rgbValue.b);
    }

  private:
    std::vector<std::string> codes_;
  };

  /**
   * @brief Returns an empty style builder.
   *
   * @return Style Empty style.
   */
  [[nodiscard]] inline Style style()
  {
    return Style{};
  }

  /**
   * @brief Returns plain text without adding style.
   *
   * @param text Input text.
   * @return std::string Unchanged text.
   */
  [[nodiscard]] inline std::string plain(std::string_view text)
  {
    return std::string(text);
  }

  /**
   * @brief Returns an empty bold style builder.
   *
   * @return Style Bold style.
   */
  [[nodiscard]] inline Style bold()
  {
    return Style{}.bold();
  }

  /**
   * @brief Applies bold style to text.
   *
   * @param text Input text.
   * @return std::string Styled text.
   */
  [[nodiscard]] inline std::string bold(std::string_view text)
  {
    return bold()(text);
  }

  [[nodiscard]] inline Style dim() { return Style{}.dim(); }
  [[nodiscard]] inline std::string dim(std::string_view text) { return dim()(text); }

  [[nodiscard]] inline Style italic() { return Style{}.italic(); }
  [[nodiscard]] inline std::string italic(std::string_view text) { return italic()(text); }

  [[nodiscard]] inline Style underline() { return Style{}.underline(); }
  [[nodiscard]] inline std::string underline(std::string_view text) { return underline()(text); }

  [[nodiscard]] inline Style blink() { return Style{}.blink(); }
  [[nodiscard]] inline std::string blink(std::string_view text) { return blink()(text); }

  [[nodiscard]] inline Style inverse() { return Style{}.inverse(); }
  [[nodiscard]] inline std::string inverse(std::string_view text) { return inverse()(text); }

  [[nodiscard]] inline Style hidden() { return Style{}.hidden(); }
  [[nodiscard]] inline std::string hidden(std::string_view text) { return hidden()(text); }

  [[nodiscard]] inline Style strikethrough() { return Style{}.strikethrough(); }
  [[nodiscard]] inline std::string strikethrough(std::string_view text) { return strikethrough()(text); }

  [[nodiscard]] inline Style black() { return Style{}.black(); }
  [[nodiscard]] inline std::string black(std::string_view text) { return black()(text); }

  [[nodiscard]] inline Style red() { return Style{}.red(); }
  [[nodiscard]] inline std::string red(std::string_view text) { return red()(text); }

  [[nodiscard]] inline Style green() { return Style{}.green(); }
  [[nodiscard]] inline std::string green(std::string_view text) { return green()(text); }

  [[nodiscard]] inline Style yellow() { return Style{}.yellow(); }
  [[nodiscard]] inline std::string yellow(std::string_view text) { return yellow()(text); }

  [[nodiscard]] inline Style blue() { return Style{}.blue(); }
  [[nodiscard]] inline std::string blue(std::string_view text) { return blue()(text); }

  [[nodiscard]] inline Style magenta() { return Style{}.magenta(); }
  [[nodiscard]] inline std::string magenta(std::string_view text) { return magenta()(text); }

  [[nodiscard]] inline Style cyan() { return Style{}.cyan(); }
  [[nodiscard]] inline std::string cyan(std::string_view text) { return cyan()(text); }

  [[nodiscard]] inline Style white() { return Style{}.white(); }
  [[nodiscard]] inline std::string white(std::string_view text) { return white()(text); }

  [[nodiscard]] inline Style gray() { return Style{}.gray(); }
  [[nodiscard]] inline std::string gray(std::string_view text) { return gray()(text); }

  [[nodiscard]] inline Style grey() { return Style{}.grey(); }
  [[nodiscard]] inline std::string grey(std::string_view text) { return grey()(text); }

  [[nodiscard]] inline Style redBright() { return Style{}.redBright(); }
  [[nodiscard]] inline std::string redBright(std::string_view text) { return redBright()(text); }

  [[nodiscard]] inline Style greenBright() { return Style{}.greenBright(); }
  [[nodiscard]] inline std::string greenBright(std::string_view text) { return greenBright()(text); }

  [[nodiscard]] inline Style yellowBright() { return Style{}.yellowBright(); }
  [[nodiscard]] inline std::string yellowBright(std::string_view text) { return yellowBright()(text); }

  [[nodiscard]] inline Style blueBright() { return Style{}.blueBright(); }
  [[nodiscard]] inline std::string blueBright(std::string_view text) { return blueBright()(text); }

  [[nodiscard]] inline Style magentaBright() { return Style{}.magentaBright(); }
  [[nodiscard]] inline std::string magentaBright(std::string_view text) { return magentaBright()(text); }

  [[nodiscard]] inline Style cyanBright() { return Style{}.cyanBright(); }
  [[nodiscard]] inline std::string cyanBright(std::string_view text) { return cyanBright()(text); }

  [[nodiscard]] inline Style whiteBright() { return Style{}.whiteBright(); }
  [[nodiscard]] inline std::string whiteBright(std::string_view text) { return whiteBright()(text); }

  [[nodiscard]] inline Style bgBlack() { return Style{}.bgBlack(); }
  [[nodiscard]] inline std::string bgBlack(std::string_view text) { return bgBlack()(text); }

  [[nodiscard]] inline Style bgRed() { return Style{}.bgRed(); }
  [[nodiscard]] inline std::string bgRed(std::string_view text) { return bgRed()(text); }

  [[nodiscard]] inline Style bgGreen() { return Style{}.bgGreen(); }
  [[nodiscard]] inline std::string bgGreen(std::string_view text) { return bgGreen()(text); }

  [[nodiscard]] inline Style bgYellow() { return Style{}.bgYellow(); }
  [[nodiscard]] inline std::string bgYellow(std::string_view text) { return bgYellow()(text); }

  [[nodiscard]] inline Style bgBlue() { return Style{}.bgBlue(); }
  [[nodiscard]] inline std::string bgBlue(std::string_view text) { return bgBlue()(text); }

  [[nodiscard]] inline Style bgMagenta() { return Style{}.bgMagenta(); }
  [[nodiscard]] inline std::string bgMagenta(std::string_view text) { return bgMagenta()(text); }

  [[nodiscard]] inline Style bgCyan() { return Style{}.bgCyan(); }
  [[nodiscard]] inline std::string bgCyan(std::string_view text) { return bgCyan()(text); }

  [[nodiscard]] inline Style bgWhite() { return Style{}.bgWhite(); }
  [[nodiscard]] inline std::string bgWhite(std::string_view text) { return bgWhite()(text); }

  [[nodiscard]] inline Style bgGray() { return Style{}.bgGray(); }
  [[nodiscard]] inline std::string bgGray(std::string_view text) { return bgGray()(text); }

  [[nodiscard]] inline Style bgGrey() { return Style{}.bgGrey(); }
  [[nodiscard]] inline std::string bgGrey(std::string_view text) { return bgGrey()(text); }

  [[nodiscard]] inline Style bgRedBright() { return Style{}.bgRedBright(); }
  [[nodiscard]] inline std::string bgRedBright(std::string_view text) { return bgRedBright()(text); }

  [[nodiscard]] inline Style bgGreenBright() { return Style{}.bgGreenBright(); }
  [[nodiscard]] inline std::string bgGreenBright(std::string_view text) { return bgGreenBright()(text); }

  [[nodiscard]] inline Style bgYellowBright() { return Style{}.bgYellowBright(); }
  [[nodiscard]] inline std::string bgYellowBright(std::string_view text) { return bgYellowBright()(text); }

  [[nodiscard]] inline Style bgBlueBright() { return Style{}.bgBlueBright(); }
  [[nodiscard]] inline std::string bgBlueBright(std::string_view text) { return bgBlueBright()(text); }

  [[nodiscard]] inline Style bgMagentaBright() { return Style{}.bgMagentaBright(); }
  [[nodiscard]] inline std::string bgMagentaBright(std::string_view text) { return bgMagentaBright()(text); }

  [[nodiscard]] inline Style bgCyanBright() { return Style{}.bgCyanBright(); }
  [[nodiscard]] inline std::string bgCyanBright(std::string_view text) { return bgCyanBright()(text); }

  [[nodiscard]] inline Style bgWhiteBright() { return Style{}.bgWhiteBright(); }
  [[nodiscard]] inline std::string bgWhiteBright(std::string_view text) { return bgWhiteBright()(text); }

  /**
   * @brief Returns a foreground RGB style.
   *
   * @param r Red channel in range [0, 255].
   * @param g Green channel in range [0, 255].
   * @param b Blue channel in range [0, 255].
   * @return Style RGB style.
   */
  [[nodiscard]] inline Style rgb(int r, int g, int b)
  {
    return Style{}.rgb(r, g, b);
  }

  /**
   * @brief Applies a foreground RGB style to text.
   *
   * @param r Red channel in range [0, 255].
   * @param g Green channel in range [0, 255].
   * @param b Blue channel in range [0, 255].
   * @param text Input text.
   * @return std::string Styled text.
   */
  [[nodiscard]] inline std::string rgb(int r, int g, int b, std::string_view text)
  {
    return rgb(r, g, b)(text);
  }

  /**
   * @brief Returns a background RGB style.
   *
   * @param r Red channel in range [0, 255].
   * @param g Green channel in range [0, 255].
   * @param b Blue channel in range [0, 255].
   * @return Style RGB background style.
   */
  [[nodiscard]] inline Style bgRgb(int r, int g, int b)
  {
    return Style{}.bgRgb(r, g, b);
  }

  /**
   * @brief Applies a background RGB style to text.
   *
   * @param r Red channel in range [0, 255].
   * @param g Green channel in range [0, 255].
   * @param b Blue channel in range [0, 255].
   * @param text Input text.
   * @return std::string Styled text.
   */
  [[nodiscard]] inline std::string bgRgb(int r, int g, int b, std::string_view text)
  {
    return bgRgb(r, g, b)(text);
  }

  /**
   * @brief Returns a foreground HEX style.
   *
   * @param value HEX value such as "#ff9900" or "f90".
   * @return Style HEX style.
   */
  [[nodiscard]] inline Style hex(const std::string &value)
  {
    return Style{}.hex(value);
  }

  /**
   * @brief Applies a foreground HEX style to text.
   *
   * @param value HEX value such as "#ff9900" or "f90".
   * @param text Input text.
   * @return std::string Styled text.
   */
  [[nodiscard]] inline std::string hex(const std::string &value, std::string_view text)
  {
    return hex(value)(text);
  }

  /**
   * @brief Returns a background HEX style.
   *
   * @param value HEX value such as "#1e1e1e" or "333".
   * @return Style HEX background style.
   */
  [[nodiscard]] inline Style bgHex(const std::string &value)
  {
    return Style{}.bgHex(value);
  }

  /**
   * @brief Applies a background HEX style to text.
   *
   * @param value HEX value such as "#1e1e1e" or "333".
   * @param text Input text.
   * @return std::string Styled text.
   */
  [[nodiscard]] inline std::string bgHex(const std::string &value, std::string_view text)
  {
    return bgHex(value)(text);
  }

} // namespace vix::chalk

#endif // VIX_CHALK_CHALK_HPP
