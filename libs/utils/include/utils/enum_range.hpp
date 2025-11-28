#pragma once

#include <iterator>
#include <type_traits>

namespace utils {

template<typename EnumType, EnumType beginVal, EnumType endVal>
class EnumRange {
public:
    using Underlying = std::underlying_type_t<EnumType>;

    class iterator {
        Underlying value_;

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = EnumType;
        using difference_type = std::ptrdiff_t;
        using pointer = EnumType*;
        using reference = EnumType&;

        iterator() : value_(static_cast<Underlying>(beginVal)) {}
        explicit iterator(Underlying start) : value_(start) {}

        iterator& operator++() {
            ++value_;
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ++value_;
            return tmp;
        }
        iterator& operator--() {
            --value_;
            return *this;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            --value_;
            return tmp;
        }
        bool operator!=(const iterator& other) const {
            return value_ != other.value_;
        }
        bool operator==(const iterator& other) const {
            return value_ == other.value_;
        }
        EnumType operator*() const {
            return static_cast<EnumType>(value_);
        }
    };

    EnumRange() :
            begin_(static_cast<Underlying>(beginVal)),
            end_(static_cast<Underlying>(endVal)) {}
    iterator begin() const {
        return iterator(begin_);
    }
    iterator end() const {
        return iterator(end_ + 1);
    }
    std::reverse_iterator<iterator> rbegin() const {
        return std::reverse_iterator<iterator>(end());
    }
    std::reverse_iterator<iterator> rend() const {
        return std::reverse_iterator<iterator>(begin());
    }

private:
    Underlying begin_, end_;
};

}  // namespace utils
