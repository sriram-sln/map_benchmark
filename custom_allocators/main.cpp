#include "fixed_allocator.h"
#include "vector"
#include <unordered_map>

using map_type_t = std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, fixed_allocator_t<std::pair<const int, int>, 10>>;

map_type_t f() {
    map_type_t m(5);
    m[5] = 6;
    return m;
}

void foo(map_type_t m) {
    std::cout << m[5] << std::endl;
}


int main() {
    // fixed_allocator_t<int> allocator_;
    // std::vector<int, fixed_allocator_t<int>> v(allocator_);
    // v.emplace_back();
    // v.emplace_back();
    // v.emplace_back();
    // v.emplace_back();
    // v.emplace_back();
    // v.emplace_back();

    // std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, fixed_allocator_t<std::pair<const int, int>, 10>> map(5);

    // // map[5] = 6;
    // // fixed_allocator_t<std::pair<const int, int>, 4096> alloc_;
    // for (int i = 0; i < 5; i++) {
    //     map[i] = i;
    // }

    auto m = f();
    std::cout << m[5] << std::endl;

    foo(std::move(m));

    return 0;
}