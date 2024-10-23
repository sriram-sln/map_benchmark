#include <optional>
#include <map>
#include <unordered_map>


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
    size_t occupancy = 0;

    void insert(const U& key, const V& value) {
        if (occupancy == CAPACITY) {
            /* Decide whether to insert */
            U max_key_val = 0;
            for (auto it = map_internal.begin(); it != map_internal.end(); it++) {
                max_key_val = std::max(max_key_val, it->first);
            }
            if (key < max_key_val) {
                /* Delete highest element if necessary */
                map_internal.erase(map_internal.find(max_key_val));
                map_internal.emplace(key, value);
            }
            
        } else {
            map_internal.emplace(key, value);
            occupancy++;  
        }
    }

    void remove(const U& key) {
        auto it = map_internal.find(key);
        if (it != map_internal.end()) {
            map_internal.erase(it);
            occupancy--;
        }
    }

    void clear() {
        map_internal.clear();   
    }
};

template<typename U, typename V, size_t CAPACITY>
struct ordered_map_wrapper : public map_base<ordered_map_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{
    std::map<U, V> map_internal;
    size_t occupancy = 0;

    void insert(const U& key, const V& value) {
        if (occupancy == CAPACITY) {
            auto it = map_internal.upper_bound(key);
            if (it != map_internal.end()) {
                map_internal.emplace(key, value);
                map_internal.erase(std::next(map_internal.rbegin()).base());
            }
        } else {
            map_internal.emplace(key, value);
            occupancy++;
        }
    }

    void remove(const U& key) {
        auto it = map_internal.find(key);
        if (it != map_internal.end()) {
            map_internal.erase(it);
            occupancy--;
        }
    }

    void clear() {
        map_internal.clear();   
    }
};