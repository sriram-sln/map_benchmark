#include <array>
#include <fstream>
#include <iostream>
#include <random>

#include "maps_impl.h"

#define n_elems 10'000
#define max_num 100'000
#define n_capacity 4096

/* 64B, 512B, 1KB, 2KB, 4KB object - random data - generate objects prior to profiling */
#define obj_size 64

using number_type = uint64_t;

using obj_type = std::array<char, obj_size>;

static size_t pos_insert = 0;
static size_t pos_delete = 0;

void batch_insert(auto& map_impl, const std::array<number_type, n_elems>& keys, const std::array<obj_type, n_elems>& values, size_t n) {
    for (size_t i = pos_insert; i < pos_insert + n; i++) {
        map_impl.insert(keys[i % n_elems], values[i % n_elems]);
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
    
    std::random_device rd{};
    std::mt19937 gen{rd()};
 
    std::normal_distribution<double> d{0.0, max_num / 4.0};
 
    auto random_int = [&d, &gen]{ return number_type(abs(std::round(d(gen)))); };


    /* Generate random numbers */
    std::array<number_type, n_elems> keys;
    for (size_t i = 0; i < n_elems; i++) {
        number_type n = random_int() % max_num;
        keys[i] = n;
    }

    /* Construct values */
    std::array<obj_type, n_elems> values;
    for (size_t i = 0; i < n_elems; i++) {
        values[i] = obj_type();
    }

    std::ofstream res;
    res.open("../results.csv", std::ios::out | std::ios::trunc);
    res << "epoch" << "," << "time_taken" << '\n';

    timespec ts_monotonic_start;
    timespec ts_monotonic_end;

    ordered_map_wrapper<number_type, obj_type, n_capacity> map;
    for (size_t epoch_no = 0; epoch_no < 20; epoch_no++) {
        /* Measure time taken to do several batch inserts and batch deletes from the map */
        std::cout << "Epoch: " << epoch_no << '\n';
        clock_gettime(CLOCK_MONOTONIC, &ts_monotonic_start);
        for (size_t i = 0; i < 1'000; i++) {        
            /* One out of every 100 repetitions should insert 3000 elements */
            /* Remaining should insert 20 and delete 10 */
            if (i % 100 == 0) {
                batch_insert(map, keys, values, 3000);
            } else {
                batch_insert(map, keys, values, 20);
                // for (auto it = map.map_internal.begin(); it != map.map_internal.end(); it++) {
                    // std::cout << it->first << std::endl;
                // }
                // std::cout << std::endl;
                batch_remove(map, keys, 10);
                // for (auto it = map.map_internal.begin(); it != map.map_internal.end(); it++) {
                    // std::cout << it->first << std::endl;
                // }
                // std::cout << std::endl;
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &ts_monotonic_end);
        /* Cast to int64 before multiplication */
        std::uint64_t time_taken = (ts_monotonic_end.tv_sec - ts_monotonic_start.tv_sec) * 1e9 + (ts_monotonic_end.tv_nsec - ts_monotonic_start.tv_nsec);
        map.clear();

        /* Append results to csv */
        res << epoch_no << "," << time_taken << '\n';
    }

    res.close();

}