#include <array>
#include <fstream>
#include <iostream>
#include <random>

#include "maps_impl.h"

#define n_elems 1'000'000
#define max_num 100'000
#define n_capacity 5

using number_type = uint64_t;
using obj_type = uint64_t;

static size_t pos_insert = 0;
static size_t pos_delete = 0;

void batch_insert(auto& map_impl, const std::array<number_type, n_elems>& v, size_t n) {
    for (size_t i = pos_insert; i < pos_insert + n; i++) {
        map_impl.insert(v[i % n_elems], number_type());
    }
    pos_insert += n;
}

void batch_remove(auto& map_impl, const std::array<number_type, n_elems>& v, size_t n) {
    for (size_t i = pos_delete; i < pos_delete + n; i++) {
        map_impl.remove(v[i % n_elems]);
    }
    pos_delete += n;
}

int main(int argc, char* argv[]) {
    
    /* Generate random numbers */
    std::array<number_type, n_elems> v;
    for (size_t i = 0; i < n_elems; i++) {
        number_type n = rand() % max_num;
        v[i] = n;
    }
    std::ofstream res;
    res.open("results.csv", std::ios::out | std::ios::trunc);
    res << "epoch" << "," << "time_taken" << '\n';

    timespec ts_monotonic_start;
    timespec ts_monotonic_end;

    unordered_map_wrapper<number_type, obj_type, n_capacity> m;
    for (size_t epoch_no = 0; epoch_no < 10; epoch_no++) {
        /* Measure time taken to do several batch inserts and batch deletes from the map */
        clock_gettime(CLOCK_MONOTONIC, &ts_monotonic_start);
        for (size_t i = 0; i < 1'000'000; i++) {
            batch_insert(m, v, 50);
            // for (auto it = m.map_internal.begin(); it != m.map_internal.end(); it++) {
            //     std::cout << it->first << " " << it->second << std::endl;
            // }
            // std::cout << std::endl;
            batch_remove(m, v, 10);
            // for (auto it = m.map_internal.begin(); it != m.map_internal.end(); it++) {
            //     std::cout << it->first << " " << it->second << std::endl;
            // }
            // std::cout << std::endl;
        }
        clock_gettime(CLOCK_MONOTONIC, &ts_monotonic_end);
        std::uint64_t time_taken = (ts_monotonic_end.tv_sec - ts_monotonic_start.tv_sec) * 1e9 + (ts_monotonic_end.tv_nsec - ts_monotonic_start.tv_nsec);
        m.clear();

        /* Append results to csv */
        res << epoch_no << "," << time_taken << '\n';
    }

}