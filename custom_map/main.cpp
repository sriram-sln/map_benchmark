#include "c_hashmap.h"
#include <iostream>
#include <memory_resource>
#include "c_flatmap.h"
#include <unordered_map>
#include <random>

// static auto global_memory_resource = std::pmr::unsynchronized_pool_resource({100'000, 512});

int main() {
    // std::pmr::polymorphic_allocator<std::pair<const int, int>> pm(&global_memory_resource);
    c_flatmap<int, int> m1;
    std::unordered_map<int, int> m2;
    
    for (int i = 0; i < 100; i++) {
        std::vector<int> keys;
        /* Insert */
        for (int j = 0; j < 100; j++) {
            int x = std::rand() % 100;
            int y = std::rand() % 100;
            m1.emplace(x, y);
            m2.emplace(x, y);
            keys.push_back(x);
        }

        /* Erase */
        // std::cout << "Erasing " << std::min<size_t>(10, keys.size()) << std::endl;
        for (int k = 0; k < std::min<size_t>(40, keys.size()); k++) {
            m1.erase(keys[k]);
            m2.erase(keys[k]);
            if (m1.find(keys[k]) != m1.end()) {
                throw std::runtime_error("Erased element still in map");
            }
        }

        /* Compare */
        for (auto it = m2.begin(); it != m2.end(); ++it) {
            if (m1.find(it->first) == m1.end() || m1[it->first] != it->second) {
                throw std::runtime_error(std::string("Mismatch because: ") + std::string(m1.find(it->first) == m1.end() ? "Element not in map" : "Element different in map"));
            }
        }

        for (auto it = m1.begin(); it != m1.end(); ++it) {
            if (m2.find(it->first) == m2.end()) {
                std::cout << it->first << std::endl;
                throw std::runtime_error("Custom has extra elems");
            }
        }

        m1.clear();
        m2.clear();
    }
    
    return 0;
}