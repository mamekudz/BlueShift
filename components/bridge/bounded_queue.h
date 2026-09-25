#pragma once

#include <cstddef>
#include <cstdint>

namespace blueshift {

// Fixed-capacity ring. Overflow policy is explicit per use site.
enum class QueueOverflowPolicy : uint8_t {
    DropNewest = 0,
    DropOldest = 1,
    KeepNewestOnly = 2 // for state snapshots: capacity effectively 1 behavior when used as such
};

template <typename T, std::size_t Capacity>
class BoundedQueue {
public:
    static_assert(Capacity > 0, "Capacity must be > 0");

    bool empty() const {
        return size_ == 0;
    }

    bool full() const {
        return size_ == Capacity;
    }

    std::size_t size() const {
        return size_;
    }

    std::size_t capacity() const {
        return Capacity;
    }

    std::uint32_t overflowCount() const {
        return overflows_;
    }

    void clear() {
        head_ = 0;
        size_ = 0;
    }

    // Returns false if item was dropped due to overflow policy.
    bool push(const T &item, QueueOverflowPolicy policy) {
        if (!full()) {
            data_[(head_ + size_) % Capacity] = item;
            ++size_;
            return true;
        }

        ++overflows_;
        switch (policy) {
        case QueueOverflowPolicy::DropNewest:
            return false;
        case QueueOverflowPolicy::DropOldest:
            head_ = (head_ + 1) % Capacity;
            data_[(head_ + size_ - 1) % Capacity] = item;
            return false;
        case QueueOverflowPolicy::KeepNewestOnly:
            clear();
            data_[0] = item;
            head_ = 0;
            size_ = 1;
            return false;
        }
        return false;
    }

    bool pop(T &out) {
        if (empty()) {
            return false;
        }
        out = data_[head_];
        head_ = (head_ + 1) % Capacity;
        --size_;
        return true;
    }

    bool peek(T &out) const {
        if (empty()) {
            return false;
        }
        out = data_[head_];
        return true;
    }

private:
    T data_[Capacity] = {};
    std::size_t head_ = 0;
    std::size_t size_ = 0;
    std::uint32_t overflows_ = 0;
};

} // namespace blueshift
