#include <optional>
#include <map>
#include <queue>
#include <unordered_map>
#include <memory_resource>
#include "custom_map/c_hashmap.h"
#include "custom_map/c_flatmap.h"
#include <absl/container/flat_hash_map.h>
#include <absl/container/btree_map.h>
//#include <boost/container/pmr/flat_map.hpp>
//#include "../Flat-Map-RB-Tree/include/dro/flat-rb-tree.hpp"

static auto global_memory_resource = std::pmr::unsynchronized_pool_resource({100'000, 512});

/* Add pmr allocator  - override operator new and keep track of allocations  - also delete */

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

    bool find(const U& key) const {
        return static_cast<T*>(this)->find(key);
    }
};

template<typename U, typename V, size_t CAPACITY>
struct unordered_map_wrapper : public map_base<unordered_map_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{
    std::pmr::unordered_map<U, V> map_internal;
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

    void insert_no_rule(const U& key, const V& value) {
        map_internal.emplace(key, value);
    }

    void clear() {
        map_internal.clear();
    }

    bool find(const U& key) const {
        return map_internal.find(key) != map_internal.end();
    }
};

// using alloc_type = std::pmr::polymorphic_allocator<std::pair<U, V>>;

template<typename U, typename V, size_t CAPACITY, typename Allocator=std::pmr::polymorphic_allocator<std::pair<const U, V>>>
struct c_hashmap_wrapper : public map_base<c_hashmap_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{

    c_hashmap<U, V, std::hash<U>, std::equal_to<U>, Allocator> map_internal;
    U largest = 0;

    c_hashmap_wrapper(Allocator allocator=std::pmr::polymorphic_allocator<std::pair<const U, V>>(&global_memory_resource)) : map_internal(allocator) {

    }

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
            map_internal.remove(key);
            if (key == this->largest) {
                this->largest = 0;
                if (map_internal.size() != 0) {
                    for (auto it = map_internal.begin(); it != map_internal.end(); ++it) {
                        this->largest = std::max(this->largest, it->first);
                    }
                }
            }
        }

    }

    void insert_no_rule(const U& key, const V& value) {
        map_internal.emplace(key, value);
    }

    void clear() {
        map_internal.clear();
    }

    bool find(const U& key) const {
        return map_internal.find(key) != map_internal.end();
    }
};


template<typename U, typename V, size_t CAPACITY, typename Allocator=std::pmr::polymorphic_allocator<std::pair<const U, V>>>
struct c_flatmap_wrapper : public map_base<c_flatmap_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{

    c_flatmap<U, V, std::hash<U>, std::equal_to<U>, Allocator> map_internal;
    U largest = 0;

    c_flatmap_wrapper(Allocator allocator=std::pmr::polymorphic_allocator<std::pair<const U, V>>(&global_memory_resource)) : map_internal(allocator) {

    }

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
            map_internal.erase(key);
            if (key == this->largest) {
                this->largest = 0;
                if (map_internal.size() != 0) {
                    for (auto it = map_internal.begin(); it != map_internal.end(); ++it) {
                        this->largest = std::max(this->largest, it->first);
                    }
                }
            }
        }

    }

    void insert_no_rule(const U& key, const V& value) {
        map_internal.emplace(key, value);
    }

    void clear() {
        map_internal.clear();
    }

    bool find(const U& key) const {
        return map_internal.find(key) != map_internal.end();
    }
};


template<typename U, typename V, size_t CAPACITY>
struct flat_hash_map_wrapper : public map_base<flat_hash_map_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{
    absl::flat_hash_map<U, V, std::hash<U>, std::equal_to<U>, std::pmr::polymorphic_allocator<std::pair<const U, V>>> map_internal;
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
                    for (auto it2 = map_internal.begin(); it2 != map_internal.end(); it2++) {
                        this->largest = std::max(this->largest, it2->first);
                    }
                }
            }
        }

    }

    void insert_no_rule(const U& key, const V& value) {
        map_internal.emplace(key, value);
    }

    void clear() {
        map_internal.clear();
    }

    bool find(const U& key) const {
        return map_internal.find(key) != map_internal.end();
    }
};

/* Ordered containers */

template<typename U, typename V, size_t CAPACITY>
struct ordered_map_wrapper : public map_base<ordered_map_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{
    std::map<U, V, std::less<U>, std::pmr::polymorphic_allocator<std::pair<const U, V>>> map_internal;

    void insert(const U& key, const V& value) {
        if (map_internal.size() == CAPACITY) {
            auto it = std::next(map_internal.rbegin()).base();
            if (key < it->first) {
                map_internal.erase(it);
                map_internal.emplace(key, value);
                
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

    void insert_no_rule(const U& key, const V& value) {
        map_internal.emplace(key, value);
    }

    void clear() {
        map_internal.clear(); 
    }

    bool find(const U& key) const {
        return map_internal.find(key) != map_internal.end();
    }
};

/* Erase takes significantly longer than in std::map */
/*
template<typename U, typename V, size_t CAPACITY>
struct flat_ordered_map_wrapper : public map_base<flat_ordered_map_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{
    boost::container::flat_map<U, V, std::less<U>, std::pmr::polymorphic_allocator<std::pair<const U, V>>> map_internal;

    void insert(const U& key, const V& value) {
        if (map_internal.size() == CAPACITY) {
            auto it = std::next(map_internal.rbegin()).base();
            if (key < it->first) {
                map_internal.erase(it);
                map_internal.emplace(key, value);
                
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

    void insert_no_rule(const U& key, const V& value) {
        map_internal.emplace(key, value);
    }

    void clear() {
        map_internal.clear(); 
    }

    bool find(const U& key) const {
        return map_internal.find(key) != map_internal.end();
    }
};
*/
template<typename U, typename V, size_t CAPACITY>
struct btree_ordered_map_wrapper : public map_base<btree_ordered_map_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{
    absl::btree_map<U, V, std::less<U>, std::pmr::polymorphic_allocator<std::pair<const U, V>>> map_internal;

    void insert(const U& key, const V& value) {
        if (map_internal.size() == CAPACITY) {
            auto it = std::next(map_internal.rbegin()).base();
            if (key < it->first) {
                map_internal.erase(it);
                map_internal.emplace(key, value);
                
            }
        } else {
            map_internal.emplace(key, value);
        }
    }

    void insert_no_rule(const U& key, const V& value) {
        map_internal.emplace(key, value);
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

    bool find(const U& key) const {
        return map_internal.find(key) != map_internal.end();
    }
};

/* Missing PMR and reverse iterator */
/*
template<typename U, typename V, size_t CAPACITY>
struct flat_rbtree_map_wrapper : public map_base<flat_rbtree_map_wrapper<U, V, CAPACITY>, U, V, CAPACITY>
{
    dro::FlatMap<U, V> map_internal;
    std::uint16_t num_elems = 0;

    void insert(const U& key, const V& value) {
        // if (num_elems != map_internal.size()) {
        //     assert(false);
        // }
        if (map_internal.size() == CAPACITY) {
            auto it = map_internal.rbegin();
            if (key < it->first) {
                auto emplace_res = map_internal.emplace(key, value);
                if (emplace_res.second) {
                    map_internal.erase(it->first);
                }
            }
        } else {
            auto emplace_res = map_internal.emplace(key, value);
            if (emplace_res.second)
                num_elems++;
        }
        // if (num_elems != map_internal.size()) {
        //     assert(false);
        // }
    }

    void insert_no_rule(const U& key, const V& value) {
        map_internal.emplace(key, value);
    }

    void remove(const U& key) {
        map_internal.erase(key);
        // if (num_elems != map_internal.size()) {
        //     assert(false);
        // }
        // auto it = map_internal.find(key);
        // if (it != map_internal.end()) {
        //     map_internal.erase(key);
        //     num_elems--;
        // }
        // if (num_elems != map_internal.size()) {
        //     assert(false);
        // }
    }

    void clear() {
        num_elems = 0;
        map_internal.clear(); 
    }

    bool find(const U& key) const {
        return map_internal.find(key) != map_internal.end();
    }
};
*/
