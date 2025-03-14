#include "c_hashmap.h"
#include <iostream>
#include <memory_resource>

static auto global_memory_resource = std::pmr::unsynchronized_pool_resource({100'000, 512});

int main() {
    std::pmr::polymorphic_allocator<std::pair<const int, int>> pm(&global_memory_resource);
    c_hashmap<int, int, std::hash<int>, std::equal_to<int>, decltype(pm)> m(pm);
    m.emplace(5, 5);
    m.remove(5);
    m.insert(5, 6);
    m[5] = 190;
    // m.clear();
    for (auto it = m.begin(); it != m.end(); ++it) {
        std::cout << it->first << " " << it->second << std::endl;
    }
    std::cout << m[5] << std::endl;
    std::cout << (m.find(5) == m.end()) << std::endl;
    return 0;
}