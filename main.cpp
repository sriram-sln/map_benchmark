#include <array>
#include <fstream>
#include <iostream>
#include <memory_resource>
#include <random>

#include "maps_impl.h"

#define seed_num 1234

#define n_elems 1'000
#define max_num 100'000
#define n_capacity 4096

/* 64B, 512B, 1KB, 2KB, 4KB object - random data - generate objects prior to profiling */
#define obj_size 64

#define N_EPOCHS 10

using number_type = uint64_t;

using obj_type = std::array<char, obj_size>;
// using obj_type = uint64_t;

static size_t pos_insert = 0;
static size_t pos_delete = 0;
static size_t pos_find = 0;

static size_t n_new_calls = 0;
static size_t n_delete_calls = 0;

bool y;

/* unordered_map_wrapper */
/* flat_hash_map_wrapper */

/* ordered_map_wrapper */
/* flat_ordered_map_wrapper */
/* flat_rbtree_map_wrapper */
/* btree_ordered_map_wrapper */

// std::pmr::polymorphic_allocator<std::pair<number_type, obj_type>> pmr_alloc(&global_memory_resource);

// c_hashmap_wrapper<number_type, obj_type, n_capacity>* map_global_ptr;
unordered_map_wrapper<number_type, obj_type, n_capacity>* map_global_ptr;
// c_flatmap_wrapper<number_type, obj_type, n_capacity>* map_global_ptr;
// flat_hash_map_wrapper<number_type, obj_type, n_capacity>* map_global_ptr;

void batch_insert(auto& map_impl, const std::array<number_type, n_elems>& keys, const std::array<obj_type, n_elems>& values, size_t n) {
    for (size_t i = pos_insert; i < pos_insert + n; i++) {
        map_impl.insert(keys[i % n_elems], values[i % n_elems]);
        // map_impl.insert(keys[i % n_elems], obj_type());
    }
    pos_insert += n;
    usleep(1000);
}

void batch_remove(auto& map_impl, const std::array<number_type, n_elems>& v, size_t n) {
    for (size_t i = pos_delete; i < pos_delete + n; i++) {
        map_impl.remove(v[i % n_elems]);
    }
    pos_delete += n;
}

void batch_find(const auto& map_impl, const std::array<number_type, n_elems>& v, size_t n) {
    for (size_t i = 0; i < n; i++) {
        y = map_impl.find(v[i % n_elems]);
    }
    pos_find += n;
}

/* Overriding memory operations */
// void * operator new(size_t size)
// {
//     std::cout << "New operator overloading " << std::endl;
//     n_new_calls++;
//     void * p = malloc(size);
//     return p;
// }
 
// void operator delete(void * p)
// {
//     n_delete_calls++;
//     std::cout << "Delete operator overloading " << std::endl;
//     free(p);
// }

struct alignas(64) special_char {
    char v;
};

int main(int argc, char* argv[]) {

    std::pmr::set_default_resource(&global_memory_resource);
    /* Use isolcpu */
    /* Seed this */
    std::random_device rd{};
    std::mt19937 gen{rd()};
    gen.seed(seed_num);
 
    std::normal_distribution<double> d{0.0, max_num / 20.0};
 
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
        for (size_t j = 0; j < obj_size; j++) {
             values[i][j] = 65 + (rand() % 26);
             // std::cout << values[i][j];
        }
        // values[i] = keys[i];
    }

    std::ofstream output_chars;
    output_chars.open("../output_chars.txt", std::ios::out | std::ios::trunc);

    std::ofstream res;
    res.open("../results.csv", std::ios::out | std::ios::trunc);
    res << "epoch," << "time_taken" <<'\n';

    timespec ts_monotonic_start;
    timespec ts_monotonic_end;

    std::remove_pointer_t<decltype(map_global_ptr)> map;
    map_global_ptr = &map;
    for (size_t epoch_no = 0; epoch_no < N_EPOCHS; epoch_no++) {

        /* 2MB object - 64B * 32768 */
        std::array<special_char, 32768> random_data;
        for (size_t i = 0; i < 32768; i++) {
            random_data[i].v = rand() % 255;
        };
        std::uint64_t tsum = 0;
        std::int64_t time_taken = 0;
        /* Measure time taken to do several batch inserts and batch deletes from the map */
        std::cout << "Epoch: " << epoch_no << '\n';
        for (size_t i = 0; i < 10'000; i++) {        
            /* One out of every 100 repetitions should insert 3000 elements */
            /* Remaining should insert 20 and delete 10 */
            clock_gettime(CLOCK_MONOTONIC, &ts_monotonic_start);
            if ((i % 1000) == 1) {
                batch_insert(map, keys, values, 3000);
            } else {
                batch_insert(map, keys, values, 10);
                // for (auto it = map.map_internal.begin(); it != map.map_internal.end(); ++it) {
                //     std::cout << it->first << std::endl;
                // }
                // std::cout << std::endl;
                batch_remove(map, keys, 10);
                // for (auto it = map.map_internal.begin(); it != map.map_internal.end(); ++it) {
                //     std::cout << it->first << std::endl;
                // }
                // std::cout << std::endl;
                /* Find */
                batch_find(map, keys, 10);
            }
            clock_gettime(CLOCK_MONOTONIC, &ts_monotonic_end);
            time_taken = (std::int64_t)(ts_monotonic_end.tv_sec - ts_monotonic_start.tv_sec) * 1e9 + (ts_monotonic_end.tv_nsec - ts_monotonic_start.tv_nsec);
            
            res << epoch_no << "," << time_taken << '\n';
            for (size_t i = 0; i < 32768; i++) {
                 tsum += random_data[i].v;
                 random_data[i].v += (map.map_internal.size() % 2 == 0);
            }
            /* Large data structure - 64 byte aligned - read one byte every 64 bytes and add to sum - refresh the data per epoch */
            // batch_insert(map, keys, values, 100);
        }
        // for (auto it = map.map_internal.begin(); it != map.map_internal.end(); ++it) {
        //    assert(map.map_internal[it->first] == it->first);
        // }
        /* Cast to int64 before multiplication */
        // std::uint64_t time_taken = (ts_monotonic_end.tv_sec - ts_monotonic_start.tv_sec) * 1e9 + (ts_monotonic_end.tv_nsec - ts_monotonic_start.tv_nsec);
        

        for (auto it = map.map_internal.begin(); it != map.map_internal.end(); ++it) {
            // std::cout << it->first;
            for (size_t i = 0; i < 50; i++) {
                size_t num = rand() % obj_size;
                output_chars << it->first << tsum << map.map_internal[it->first][num];
                // std::cout << it->second[i];
            }
        }
        
        map.clear();
        /* Append results to csv */
        // res << epoch_no << "," << time_taken << "," << n_new_calls << "," << n_delete_calls << '\n';
    }

    res.close();
    output_chars.close();

}
