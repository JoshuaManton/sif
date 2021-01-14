#pragma once

#include <atomic>

struct Spinlock {
    std::atomic_flag flag;

    Spinlock() {
        flag.clear();
    }

    Spinlock(const Spinlock &tm) = delete;
    Spinlock(Spinlock &&tm) = delete;
    Spinlock& operator=(const Spinlock &tm) = delete;
    Spinlock& operator=(Spinlock && tm) = delete;

    void lock() {
        while(flag.test_and_set(std::memory_order_acquire)) { }
    }

    bool try_lock() {
        return flag.test_and_set(std::memory_order_acquire) == false;
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }
};