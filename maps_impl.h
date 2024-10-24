#include <optional>
#include <map>
#include <queue>
#include <unordered_map>
#include <absl/container/flat_hash_map.h>
#include <absl/container/btree_map.h>
#include <boost/container/flat_map.hpp>

template<typename T, typename U, typename V, size_t CAPACITY>
struct map_base
{
    void insert(const U& key, const V& value) {
        static_cast<T*>(this)->insert(key, value);
    }

    void remove(const U& key) {
        static_cast<T*>(this)->remove(key);
    }

    void clear() {
        static_cast<T*>(this)->clear();
    }
};

template<typename U, typename V, size_t CAPACITY>
struct unordered_map_wrapper : public map_base<unordered_map_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{
    std::unordered_map<U, V> map_internal;
    U largest = 0;

    void insert(const U& key, const V& value) {
        if (map_internal.size() == CAPACITY) {
            /* Decide whether to insert */
            if (key < largest) {
                map_internal.emplace(key, value);
                this->remove(largest);
            }
            
        } else {
            map_internal.emplace(key, value);
            this->largest = std::max(this->largest, key);
        }
    }

    void remove(const U& key) {
        auto it = map_internal.find(key);
        if (it != map_internal.end()) {
            map_internal.erase(it);
            if (key == this->largest) {
                this->largest = 0;
                if (map_internal.size() != 0) {
                    for (auto it = map_internal.begin(); it != map_internal.end(); it++) {
                        this->largest = std::max(this->largest, it->first);
                    }
                }
            }
        }

    }

    void clear() {
        map_internal.clear();
    }
};

template<typename U, typename V, size_t CAPACITY>
struct flat_hash_map_wrapper : public map_base<flat_hash_map_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{
    absl::flat_hash_map<U, V> map_internal;
    U largest = 0;

    void insert(const U& key, const V& value) {
        if (map_internal.size() == CAPACITY) {
            /* Decide whether to insert */
            if (key < largest) {
                map_internal.emplace(key, value);
                this->remove(largest);
            }
            
        } else {
            map_internal.emplace(key, value);
            this->largest = std::max(this->largest, key);
        }
    }

    void remove(const U& key) {
        auto it = map_internal.find(key);
        if (it != map_internal.end()) {
            map_internal.erase(it);
            if (key == this->largest) {
                this->largest = 0;
                if (map_internal.size() != 0) {
                    for (auto it = map_internal.begin(); it != map_internal.end(); it++) {
                        this->largest = std::max(this->largest, it->first);
                    }
                }
            }
        }

    }

    void clear() {
        map_internal.clear();
    }
};

/* Ordered containers */

template<typename U, typename V, size_t CAPACITY>
struct ordered_map_wrapper : public map_base<ordered_map_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{
    std::map<U, V> map_internal;

    void insert(const U& key, const V& value) {
        if (map_internal.size() == CAPACITY) {
            auto it = map_internal.upper_bound(key);
            if (it != map_internal.end()) {
                map_internal.emplace(key, value);
                map_internal.erase(std::next(map_internal.rbegin()).base());
            }
        } else {
            map_internal.emplace(key, value);
        }
    }

    void remove(const U& key) {
        auto it = map_internal.find(key);
        if (it != map_internal.end()) {
            map_internal.erase(it);
        }
    }

    void clear() {
        map_internal.clear(); 
    }
};

template<typename U, typename V, size_t CAPACITY>
struct flat_ordered_map_wrapper : public map_base<flat_ordered_map_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{
    boost::container::flat_map<U, V> map_internal;

    void insert(const U& key, const V& value) {
        if (map_internal.size() == CAPACITY) {
            auto it = map_internal.upper_bound(key);
            if (it != map_internal.end()) {
                map_internal.emplace(key, value);
                map_internal.erase(std::next(map_internal.rbegin()).base());
            }
        } else {
            map_internal.emplace(key, value);
        }
    }

    void remove(const U& key) {
        auto it = map_internal.find(key);
        if (it != map_internal.end()) {
            map_internal.erase(it);
        }
    }

    void clear() {
        map_internal.clear(); 
    }
};

template<typename U, typename V, size_t CAPACITY>
struct btree_ordered_map_wrapper : public map_base<btree_ordered_map_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{
    absl::btree_map<U, V> map_internal;

    void insert(const U& key, const V& value) {
        if (map_internal.size() == CAPACITY) {
            auto it = map_internal.upper_bound(key);
            if (it != map_internal.end()) {
                map_internal.emplace(key, value);
                map_internal.erase(std::next(map_internal.rbegin()).base());
            }
        } else {
            map_internal.emplace(key, value);
        }
    }

    void remove(const U& key) {
        auto it = map_internal.find(key);
        if (it != map_internal.end()) {
            map_internal.erase(it);
        }
    }

    void clear() {
        map_internal.clear(); 
    }
};