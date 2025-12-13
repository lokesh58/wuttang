#pragma once

#include <array>
#include <cassert>
#include <cstddef>

#include "chess/move.hpp"

namespace chess {

class MoveList {
    static constexpr std::size_t MAX_MOVES = 256;

public:
    // Modern C++ iterator traits
    using value_type = Move;
    using iterator = Move*;
    using const_iterator = const Move*;

    MoveList() noexcept = default;

    void push_back(const Move& move) noexcept {
        assert(count_ < MAX_MOVES);
        moves_[count_++] = move;
    }

    void pop_back() noexcept {
        assert(count_ > 0);
        count_--;
    }

    // Standard container accessors
    [[nodiscard]]
    std::size_t size() const noexcept {
        return count_;
    }
    [[nodiscard]]
    bool empty() const noexcept {
        return count_ == 0;
    }

    // Iterators allow you to use: for (const auto& move : move_list)
    [[nodiscard]]
    iterator begin() noexcept {
        return moves_.data();
    }
    [[nodiscard]]
    iterator end() noexcept {
        return moves_.data() + count_;
    }
    [[nodiscard]]
    const_iterator begin() const noexcept {
        return moves_.data();
    }
    [[nodiscard]]
    const_iterator end() const noexcept {
        return moves_.data() + count_;
    }

    // Random access
    [[nodiscard]]
    Move& operator[](std::size_t index) noexcept {
        return moves_[index];
    }
    [[nodiscard]]
    const Move& operator[](std::size_t index) const noexcept {
        return moves_[index];
    }

    // Easy clear for reuse
    void clear() noexcept {
        count_ = 0;
    }

private:
    std::array<Move, MAX_MOVES> moves_;
    std::size_t count_ = 0;
};

}  // namespace chess
