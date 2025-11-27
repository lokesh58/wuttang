#include "chess/square.hpp"

#include <string>

namespace chess {

std::string to_string(Square s) noexcept {
    const auto file = get_square_file(s);
    const auto rank = get_square_rank(s);

    std::string str;
    str += static_cast<char>('a' + static_cast<int>(file));
    str += static_cast<char>('1' + static_cast<int>(rank));

    return str;
}

}  // namespace chess
