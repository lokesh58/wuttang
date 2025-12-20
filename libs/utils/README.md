# Wuttang Utilities Library

A collection of generic, modern C++23 utility headers used throughout the Wuttang project.

## Components

### `enum_flags.hpp`

Provides type-safe bitwise operations for scoped enums. Enables using `enum class` as flags.

**Usage:**

```cpp
#include <wuttang/utils/enum_flags.hpp>

enum class MyFlags : uint8_t {
    None = 0,
    FlagA = 1 << 0,
    FlagB = 1 << 1
};
// Enable bitwise operators
template<> struct wuttang::utils::EnableBitMaskOperators<MyFlags> : std::true_type {};

MyFlags f = MyFlags::FlagA | MyFlags::FlagB;
```

### `enum_range.hpp`

Utilities for iterating over the range of an enum's values.

**Usage:**

```cpp
#include <wuttang/utils/enum_range.hpp>

for (auto file : wuttang::utils::EnumRange<File, File::FILE_A, File::FILE_H>()) {
    // iterate through files A to H
}
```

### `enum_shift.hpp`

Provides arithmetic shift operations for enums, useful for grid-based logic (like chess boards).

**Usage:**

```cpp
#include <wuttang/utils/enum_shift.hpp>

Square sq = Square::A1;
Square next_sq = wuttang::utils::shift(sq, 1); // B1
```
