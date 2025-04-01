#include <stddef.h>
#include <iostream>
#include <memory>
#include <numeric>
#include <assert.h>
#include <concepts>

template<typename T>
concept memory_fifo_compatible_t = requires {
    requires !std::is_pointer_v<T>;
};

template<typename T, size_t n_elems>
struct memory_bank {
    struct memory_slot_t {
        unsigned char data[sizeof(T)];
    };

    size_t in_{0};
    size_t out_{0};
    size_t capacity_{n_elems};

    unsigned char bank[sizeof(T) * n_elems];
    size_t free_list_[n_elems];

    memory_bank() {
        // std::iota(free_list_, free_list_ + (n_elems * sizeof(size_t)), 0);
        for (size_t i = 0; i < n_elems; i++) {
            free_list_[i] = i;
        }
    }

    T* get_slot() {
        if (out_ == in_ - 1 || (out_ == capacity_ && in_ == 0))
            throw 505;
        size_t ind = out_++;
        if (out_ == capacity_)
            out_ = 0;
        return reinterpret_cast<T*>(&bank[free_list_[ind] * sizeof(T)]);
    }

    void free_slot(T* slot) {
        if (in_ == out_)
            throw 506;
        size_t offset = (reinterpret_cast<unsigned char*>(slot) - bank) / sizeof(T);
        free_list_[in_++] = offset;
        if (in_ == capacity_)
            in_ = 0;
    }
};

template<typename T, size_t n_elems, bool alloc_one_only=false>
struct fixed_allocator_t {

    using value_type = T;

    template <class U> 
    struct rebind { using other = fixed_allocator_t<U, n_elems>; };

    memory_bank<T, n_elems> bank;

    size_t x{0};

    fixed_allocator_t() {}

    template<typename Alloc_Other>
    fixed_allocator_t(Alloc_Other) {}

    T* allocate(size_t n) {
        if constexpr (!std::is_pointer_v<T>){
            assert(n == 1);
            x++;
            return bank.get_slot();
        }
        assert(x <= n_elems);
        return static_cast<T*>(malloc(sizeof(T) * n));
    }

    void deallocate(T* p, size_t n) {
        if constexpr (!std::is_pointer_v<T>){
            assert(n == 1);
            x--;
            return bank.free_slot(p);
        }
        assert(x >= 0);
        free(p);
    }

    // ~fixed_allocator_t() {}

};