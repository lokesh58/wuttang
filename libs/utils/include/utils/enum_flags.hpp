#pragma once

#include <type_traits>

namespace utils {

/**
 * @brief Trait to enable bitwise operators for an enum class.
 *
 * Specialize this struct for your enum class and set value = true.
 * Example:
 * template<>
 * struct enable_bitmask_operators<MyEnum> {
 *     static constexpr bool value = true;
 * };
 */
template<typename E>
struct enable_bitmask_operators {
    static constexpr bool value = false;
};

}  // namespace utils

template<typename E>
constexpr
    typename std::enable_if<utils::enable_bitmask_operators<E>::value, E>::type
    operator|(E lhs, E rhs) {
    using underlying = typename std::underlying_type<E>::type;
    return static_cast<E>(
        static_cast<underlying>(lhs) | static_cast<underlying>(rhs)
    );
}

template<typename E>
constexpr
    typename std::enable_if<utils::enable_bitmask_operators<E>::value, E>::type
    operator&(E lhs, E rhs) {
    using underlying = typename std::underlying_type<E>::type;
    return static_cast<E>(
        static_cast<underlying>(lhs) & static_cast<underlying>(rhs)
    );
}

template<typename E>
constexpr
    typename std::enable_if<utils::enable_bitmask_operators<E>::value, E>::type
    operator^(E lhs, E rhs) {
    using underlying = typename std::underlying_type<E>::type;
    return static_cast<E>(
        static_cast<underlying>(lhs) ^ static_cast<underlying>(rhs)
    );
}

template<typename E>
constexpr
    typename std::enable_if<utils::enable_bitmask_operators<E>::value, E>::type
    operator~(E lhs) {
    using underlying = typename std::underlying_type<E>::type;
    return static_cast<E>(~static_cast<underlying>(lhs));
}

template<typename E>
constexpr
    typename std::enable_if<utils::enable_bitmask_operators<E>::value, E&>::type
    operator|=(E& lhs, E rhs) {
    lhs = lhs | rhs;
    return lhs;
}

template<typename E>
constexpr
    typename std::enable_if<utils::enable_bitmask_operators<E>::value, E&>::type
    operator&=(E& lhs, E rhs) {
    lhs = lhs & rhs;
    return lhs;
}

template<typename E>
constexpr
    typename std::enable_if<utils::enable_bitmask_operators<E>::value, E&>::type
    operator^=(E& lhs, E rhs) {
    lhs = lhs ^ rhs;
    return lhs;
}
