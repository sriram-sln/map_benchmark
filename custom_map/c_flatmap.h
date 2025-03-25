#include <memory>
#include <iostream>
#include <cstring>

#define default_n_buckets 1000

template<typename U, typename V, typename Hash = std::hash<U>, typename KeyEqual = std::equal_to<U>, typename Allocator = std::allocator<std::pair<const U, V>>>
struct c_flatmap {

    using value_type = std::pair<const U, V>;

    struct obj_meta_t {
        U key_;
        bool empty_;
        bool tombstone_;
        size_t offset_;
        size_t prev_;
        size_t next_;
        obj_meta_t () : offset_(0), empty_(true), tombstone_(false) {}
        obj_meta_t(const U key, size_t offset) : key_(key), offset_(offset), empty_(false), tombstone_(false) {}
    };

    using obj_value_allocator_t = Allocator; // std::allocator_traits<Allocator>::template rebind_alloc<V>;
    using obj_meta_allocator_t = std::allocator_traits<Allocator>::template rebind_alloc<obj_meta_t>;

    obj_meta_t* obj_meta_;
    value_type* objs_;
    Allocator allocator_;
    obj_value_allocator_t obj_value_allocator_;
    obj_meta_allocator_t obj_meta_allocator_;
    size_t n_buckets_;
    size_t objs_offset_;
    Hash hash_;
    KeyEqual key_equal_;
    size_t begin_;
    size_t size_;

    struct Iterator {
        value_type* objs_base_;
        obj_meta_t* obj_meta_base_;
        size_t obj_meta_offset_;
        size_t n_buckets_;
        
        Iterator& operator++() {
            if (obj_meta_offset_ < n_buckets_)
                obj_meta_offset_++;
            while (obj_meta_offset_ < n_buckets_ && (obj_meta_base_[obj_meta_offset_].empty_ || obj_meta_base_[obj_meta_offset_].tombstone_)) {
                obj_meta_offset_++;
            }
            return *this;
        }

        value_type& operator*() {
            return objs_base_[obj_meta_base_[obj_meta_offset_].offset_];
        }

        value_type* operator->() {
            return &objs_base_[obj_meta_base_[obj_meta_offset_].offset_];
        }

        friend bool operator==(const Iterator& a, const Iterator& b) {
            return a.obj_meta_base_ == b.obj_meta_base_ && a.obj_meta_offset_ == b.obj_meta_offset_;
        }

        friend bool operator!=(const Iterator& a, const Iterator& b) {
            return a.obj_meta_base_ != b.obj_meta_base_ || a.obj_meta_offset_ != b.obj_meta_offset_;
        }

        Iterator(obj_meta_t* obj_meta_base, value_type* objs_base, size_t obj_meta_offset, size_t n_buckets) : obj_meta_base_(obj_meta_base), objs_base_(objs_base), obj_meta_offset_(obj_meta_offset), n_buckets_(n_buckets) {}
    };

    Iterator begin() const {return Iterator(obj_meta_, objs_, begin_, n_buckets_);}
    Iterator end() const {return Iterator(obj_meta_, objs_, n_buckets_, n_buckets_);}

    c_flatmap(Allocator allocator=std::allocator<std::pair<const U, V>>(), size_t n_buckets=default_n_buckets) : allocator_(allocator), obj_meta_allocator_(allocator), obj_value_allocator_(allocator), n_buckets_(n_buckets), begin_(n_buckets), objs_offset_(0) {
        obj_meta_t* obj_meta_storage = obj_meta_allocator_.allocate(n_buckets_);
        obj_meta_ = new (obj_meta_storage) obj_meta_t[n_buckets_];
        objs_ = obj_value_allocator_.allocate(n_buckets);
    }

    template<typename ...Args>
    Iterator emplace(const U key, Args&& ...args) {
        size_t bucket_num;
        while (true) {
            bucket_num = hash_(key) % n_buckets_;
            while (bucket_num < n_buckets_ && !obj_meta_[bucket_num].empty_) {
                if (!obj_meta_[bucket_num].tombstone_ && key_equal_(objs_[obj_meta_[bucket_num].offset_].first, key))
                    return Iterator(obj_meta_, objs_, bucket_num, n_buckets_);
                bucket_num++;
            }
            if (bucket_num == n_buckets_)
                rehash();
            else
                break;
        }
        if (bucket_num < begin_)
            begin_ = bucket_num;
        new (&obj_meta_[bucket_num]) obj_meta_t(key, objs_offset_);
        new (&objs_[objs_offset_++]) std::pair<const U, V>(std::piecewise_construct, std::make_tuple(key), std::make_tuple(std::forward<Args...>(args...)));
        size_++;
        return Iterator(obj_meta_, objs_, bucket_num, n_buckets_);
    }

    void erase(const U key) {
        Iterator it = find(key);
        if (it == end())
            return;

        size_t offset = it.obj_meta_offset_;
        if (obj_meta_[offset].tombstone_) {
            throw std::runtime_error("Attempted to erase tombstone");
        }
        obj_meta_[offset].tombstone_ = true;
        (*it).~value_type();
        reset_begin();
        size_--;
    }

    void rehash() {
        rehash(n_buckets_ * 2);
    }

    void rehash(size_t new_n_buckets) {
        obj_meta_t* new_obj_meta = obj_meta_allocator_.allocate(new_n_buckets);
        value_type* new_objs = obj_value_allocator_.allocate(new_n_buckets);
        new (new_obj_meta) obj_meta_t[new_n_buckets];
        // for (size_t meta_offset = 0; meta_offset < new_n_buckets; meta_offset++) {
        //     new_obj_meta[meta_offset].empty_ = true;
        //     new_obj_meta[meta_offset].tombstone_ = false;
        // }
        size_t new_begin = new_n_buckets;
        size_t new_offset = 0;
        for (auto it = begin(); it != end(); ++it) {
            auto hash = hash_(it->first);
            auto offset = obj_meta_[it.obj_meta_offset_].offset_;
            auto new_bucket_num = hash % new_n_buckets;
            while (new_bucket_num < new_n_buckets && !new_obj_meta[new_bucket_num].empty_) {
                new_bucket_num++;
            }
            if (new_bucket_num == new_n_buckets) {
                obj_meta_allocator_.deallocate(new_obj_meta, new_n_buckets);
                obj_value_allocator_.deallocate(new_objs, new_n_buckets);
                rehash(new_n_buckets * 2);
                return;
            }
            new_begin = std::min(new_begin, new_bucket_num);
            new_obj_meta[new_bucket_num].offset_ = new_offset++;
            new_obj_meta[new_bucket_num].key_ = it->first;
            new_obj_meta[new_bucket_num].empty_ = false;
        }

        new_offset = 0;

        for (auto it = begin(); it != end(); ++it) {
            auto offset = obj_meta_[it.obj_meta_offset_].offset_;
            new (&new_objs[new_offset++]) value_type(objs_[offset]);
            objs_[offset].~value_type();
        }
        
        obj_value_allocator_.deallocate(objs_, n_buckets_);
        obj_meta_allocator_.deallocate(obj_meta_, n_buckets_);

        objs_ = new_objs;
        obj_meta_ = new_obj_meta;
        n_buckets_ = new_n_buckets;
        begin_ = new_begin;
        objs_offset_ = new_offset;
    }

    void reset_begin() {
        for (size_t offset = 0; offset < n_buckets_; offset++) {
            if (!obj_meta_[offset].empty_ && !obj_meta_[offset].tombstone_) {
                begin_ = offset;
                return;
            }
        }
    }

    Iterator find(const U key) const {
        size_t bucket_num = hash_(key) % n_buckets_;
        while (bucket_num < n_buckets_ && (!obj_meta_[bucket_num].empty_) && (obj_meta_[bucket_num].tombstone_ || !key_equal_(obj_meta_[bucket_num].key_, key))) {
            bucket_num++;
        }
        if (bucket_num == n_buckets_ || obj_meta_[bucket_num].empty_ || (obj_meta_[bucket_num].tombstone_ || !key_equal_(obj_meta_[bucket_num].key_, key)))
            return end();
        return Iterator(obj_meta_, objs_, bucket_num, n_buckets_);
    }

    V& operator[](const U key) {
        auto it = find(key);
        if (it == end()){
            it = emplace(key, V());
        }
        // emplace(key);
        return it->second;
    }

    void clear() {
        for (size_t i = 0; i < n_buckets_; i++) {
            if (!obj_meta_[i].empty_ && !obj_meta_[i].tombstone_) {
                size_t offset = obj_meta_[i].offset_;
                objs_[offset].~value_type();
            }
            obj_meta_[i].empty_ = true;
            obj_meta_[i].tombstone_ = false;
        }
        objs_offset_ = 0;
        begin_ = n_buckets_;
    }

    size_t size() {
        return size_;
    }

};