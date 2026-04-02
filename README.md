# vix/chalk

Terminal string styling for C++.

**Header-only. Expressive. ANSI-based.**

---

## Download

https://vixcpp.com/registry/pkg/vix/chalk

---

## Overview

`vix/chalk` provides a simple and expressive API to style terminal output.

It supports:

- foreground colors
- background colors
- text styles
- RGB colors
- HEX colors

It is designed to be:

- minimal
- predictable
- easy to use
- zero overhead

---

## Why vix/chalk?

Styling terminal output is often:

- verbose
- repetitive
- hard to read
- error-prone

Using raw ANSI codes leads to:

- poor readability
- duplicated logic
- hard maintenance

`vix/chalk` provides:

- a clean API
- composable styles
- readable code

---

## Installation

### Using Vix

```bash
vix add @vix/chalk
vix install
```

### Manual

```bash
git clone https://github.com/vixcpp/chalk.git
```

Add `include/` to your project.

---

## Basic Usage

```cpp
#include <vix/chalk/chalk.hpp>
#include <iostream>

int main()
{
  using namespace vix::chalk;

  std::cout << red("error") << '\n';
  std::cout << green("success") << '\n';
}
```

---

## Chaining Styles

```cpp
std::cout << red().bold()("fatal error") << '\n';
std::cout << blue().underline()("important") << '\n';
```

---

## Background Colors

```cpp
std::cout << bgRed().white().bold()(" ERROR ") << '\n';
std::cout << bgGreen().black()(" OK ") << '\n';
```

---

## RGB Colors

```cpp
std::cout << rgb(255, 153, 0)("orange") << '\n';
std::cout << bgRgb(30, 30, 30).white()("panel") << '\n';
```

---

## HEX Colors

```cpp
std::cout << hex("#ff9900")("brand") << '\n';
std::cout << bgHex("#1e1e1e").white()("title") << '\n';
```

---

## Utilities

### Strip ANSI

```cpp
std::string clean = strip_ansi(red().bold()("error"));
```

### Enable / Disable

```cpp
setEnabled(false);
setEnabled(true);
```

---

## Execution Model

- styles are immutable
- chaining builds a new style
- application happens at call time
- no global state mutation

---

## Complexity

| Operation | Complexity |
|----------|-----------|
| style creation | O(1) |
| chaining | O(1) |
| apply style | O(n) |

---

## Design Philosophy

- minimal API
- explicit behavior
- composable styles
- no hidden magic
- header-only simplicity

---

## Tests

```bash
vix build
vix test
```

---

## License

MIT License
Copyright (c) Gaspard Kirira

