#include <memory>
#include <iostream>

#define c_hash_factor 1000

template<typename U, typename V, typename Hash = std::hash<U>, typename KeyEqual = std::equal_to<U>, typename Allocator = std::allocator<std::pair<const U, V>>>
struct c_hashmap {

    using value_type = std::pair<const U, V>;
    

    struct ListNode {
        value_type* key_value;
        ListNode* next;
        
        template<typename ...Args>
        ListNode(value_type* key_value) : key_value(key_value), next(nullptr) {
            
        }

        ~ListNode() {
            delete key_value;
        }
    };

    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        // using value_type = value_type;
        using pointer = value_type*;
        using reference = value_type&;

        ListNode* m_ptr;
        ListNode*const* buckets_;
        size_t bucket_num_;

        Iterator(ListNode* const* buckets, size_t bucket_num, ListNode* ln_ptr=nullptr) : buckets_(buckets), bucket_num_(bucket_num), m_ptr(ln_ptr ? ln_ptr : buckets[bucket_num])  {}

        reference operator*() const { return *(m_ptr->key_value); }
        pointer operator->() { return (m_ptr->key_value); }

        // Prefix increment
        Iterator& operator++() {
            m_ptr = m_ptr->next;
            // std::cout << bucket_num_ << "yes" << std::endl;
            for (;;) {
                if (m_ptr || bucket_num_ == c_hash_factor)
                    break;
                if (!m_ptr) {
                    m_ptr = buckets_[++bucket_num_];
                }
            }
            // std::cout << bucket_num_ << "yes" << std::endl;
            return *this; 
        }  

        // Postfix increment
        // Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };  
    };

    using listnode_allocator_t = std::allocator_traits<Allocator>::template rebind_alloc<ListNode>;

    ListNode* buckets[c_hash_factor + 1];
    Allocator allocator_;
    listnode_allocator_t listnode_allocator_;
    Hash hash_;
    KeyEqual key_equal_;

    size_t size_;

    Iterator begin() const {
        size_t bnum = 0;
        for (size_t i = 0; i <= c_hash_factor; i++) {
            bnum = i;
            if (buckets[i])
                break;
        }
        return Iterator(static_cast<ListNode*const*>(buckets), bnum);
    }

    Iterator end() const {
        return Iterator(static_cast<ListNode*const*>(buckets), c_hash_factor);
    }

    c_hashmap(Allocator allocator=std::allocator<std::pair<const U, V>>()) : allocator_(allocator), listnode_allocator_(allocator) {
        std::memset(buckets, 0, sizeof(buckets));
    }

    template<typename ...Args>
    void emplace(const U key, Args&& ...args) {
        if (find_key(key))
            return;

        using traits_t2 = std::allocator_traits<decltype(allocator_)>;

        value_type* kv = allocator_.allocate(1);
        new ((void*)kv) value_type(std::piecewise_construct, std::make_tuple(key), std::make_tuple(std::forward<Args>(args)...));
        // ListNode* ln = new ListNode(kv);
        ListNode* ln = listnode_allocator_.allocate(1);
        new ((void*)ln) ListNode(kv);
        // allocator_.construct(ln, std::forward(key, args...));

        auto bucket_num = hash_(key) % c_hash_factor;
        insertNode(&buckets[bucket_num], ln);
    }

    void insert(const U key, V& value) {
        emplace(key, value);
    }

    void insert(const U key, V&& value) {
        ListNode* pos = find_key(key);
        if (pos != nullptr) {
            pos->key_value->second = value;
            return;
        }
        emplace(key, value);
    }

    void remove(const U key) {
        ListNode** bucket = &buckets[hash_(key) % c_hash_factor];
        ListNode* pos = *bucket;
        ListNode* prev = pos;
        while (pos && !(key_equal_(pos->key_value->first, key))) {
            prev = pos;
            pos = pos->next;
        }
        if (!pos)
            return;

        if (pos != prev) {
            prev->next = pos->next;
        } else {
            *bucket = nullptr;
        }
        listnode_allocator_.deallocate(pos, 1);
        size_--;
    }

    void insertNode(ListNode** dest, ListNode* ln) {
        if (!(*dest)) {
            *dest = ln;
            return;
        }
        ListNode* prev = *dest;
        ListNode* cur = *dest;
        while (cur) {
            prev = cur;
            cur = cur->next;
        }
        prev->next = ln;
        size_++;
    }

    ListNode* find_key(const U key) const {
        ListNode* pos = buckets[hash_(key) % c_hash_factor];
        while (pos && !(key_equal_(pos->key_value->first, key))) {
            pos = pos->next;
        }
        return pos;
    }

    V& operator[](const U key) {
        ListNode* pos = find_key(key);
        if (!pos) {
            emplace(key);
            pos = find_key(key);
        }
        return pos->key_value->second;
    }

    Iterator find(const U key) const {
        ListNode* pos = find_key(key);
        if (pos) {
            return Iterator(static_cast<ListNode*const*>(buckets), hash_(key) % c_hash_factor, pos);
        }
        // for (auto it = begin(); it != end(); ++it) {
        //     if (key_equal_(it->first, key))
        //         return it;
        // }
        return end();
    }

    void clear() {
        auto it = begin();
        ln_dealloc(it);
        // for (size_t i = 0; i < c_hash_factor; i++) {
        //     buckets[i] = nullptr;
        // }
        std::memset(buckets, 0, c_hash_factor);
    }

    void ln_dealloc(Iterator it) {
        if (it != end()) {
            auto tmp = it;
            ln_dealloc(++it);
            listnode_allocator_.deallocate(tmp.m_ptr, 1);
        }
    }

    size_t size() {
        return size_;
    }
};