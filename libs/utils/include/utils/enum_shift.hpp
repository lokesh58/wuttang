#pragma once

#include <concepts>
#include <type_traits>

namespace utils {

template<typename T>
struct EnumTraits;

template<typename T>
concept Shiftable = requires {
    typename utils::EnumTraits<T>::ArithmeticType;
    // Ensure the underlying type can be converted to the arithmetic type
    requires std::convertible_to<
        std::underlying_type_t<T>,
        typename utils::EnumTraits<T>::ArithmeticType
    >;

    { utils::EnumTraits<T>::min } -> std::convertible_to<T>;
    { utils::EnumTraits<T>::max } -> std::convertible_to<T>;
    { utils::EnumTraits<T>::sentinel } -> std::convertible_to<T>;
};

template<Shiftable T>
inline constexpr T shift(
    T value,
    typename utils::EnumTraits<T>::ArithmeticType delta
) noexcept {
    using Underlying = std::underlying_type_t<T>;
    using Arithmetic = typename utils::EnumTraits<T>::ArithmeticType;
    using Traits = utils::EnumTraits<T>;

    // Perform arithmetic using the specified ArithmeticType
    const auto val_arith =
        static_cast<Arithmetic>(static_cast<Underlying>(value));
    const auto new_val = val_arith + delta;

    // Compare against min and max (casted to Arithmetic type via Underlying)
    const auto min_arith =
        static_cast<Arithmetic>(static_cast<Underlying>(Traits::min));
    const auto max_arith =
        static_cast<Arithmetic>(static_cast<Underlying>(Traits::max));

    if (new_val < min_arith || new_val > max_arith) {
        return Traits::sentinel;
    }

    return static_cast<T>(static_cast<Underlying>(new_val));
}

}  // namespace utils
