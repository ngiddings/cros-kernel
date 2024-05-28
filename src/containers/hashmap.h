#ifndef HASHMAP_H
#define HASHMAP_H

#include "pair.h"
#include "string.h"
#include "vector.h"

template <typename K, typename V>
class hashmap {
   public:
    hashmap();
    ~hashmap();

    int size() const;
    bool empty() const;

    void insert(const K& key, const V& value);
    void remove(const K& key);
    const V& get(const K& key) const;
    bool contains(const K& key) const;

    void clear();

    V& operator[](const K& key) const;

   private:
    static constexpr int hash_prime               = 5381;
    static constexpr int default_capacity         = 16;
    static constexpr double load_factor_threshold = 0.75;
    static V default_value;

    vector<pair<K, V>>* table;
    int _size;
    int _capacity;

    int hash(const int& key) const;
    int hash(const double& key) const;
    int hash(const string& key) const;

    void rehash_table();
};

template <typename K, typename V>
V hashmap<K, V>::default_value = V();

template <typename K, typename V>
hashmap<K, V>::hashmap() {
    _size     = 0;
    _capacity = default_capacity;
    table     = new vector<pair<K, V>>[_capacity];
    for (int i = 0; i < _capacity; i++)
        table[i] = vector<pair<K, V>>();
}

template <typename K, typename V>
hashmap<K, V>::~hashmap() {
    if (table != nullptr)
        // delete[] array;    // need to overload delete[] to not call deconstructors of array object, std implementation has unwanted side effects
        table = nullptr;
}

template <typename K, typename V>
int hashmap<K, V>::size() const {
    return _size;
}

template <typename K, typename V>
bool hashmap<K, V>::empty() const {
    return size == 0;
}

template <typename K, typename V>
void hashmap<K, V>::insert(const K& key, const V& value) {
    double load_factor = (double)_size / _capacity;
    if (load_factor > load_factor_threshold)
        rehash_table();
    int index = hash(key) % _capacity;
    table[index].push_back(pair(key, value));
    _size++;
}

template <typename K, typename V>
void hashmap<K, V>::remove(const K& key) {
    int index = hash(key) % _capacity;
    for (int i = 0; i < table[index].size(); i++) {
        if (table[index][i].first == key) {
            table[index] = V();
            _size--;
            break;
        }
    }
}

template <typename K, typename V>
const V& hashmap<K, V>::get(const K& key) const {
    int index = hash(key) % _capacity;
    for (int i = 0; i < table[index].size(); i++) {
        if (table[index][i].first == key)
            return table[index][i].second;
    }
    return default_value;
}

template <typename K, typename V>
bool hashmap<K, V>::contains(const K& key) const {
    int index = hash(key) % table.capacity();
    for (int i = 0; i < table[index].size(); i++) {
        if (table[index][i].first == key)
            return true;
    }
    return false;
}

template <typename K, typename V>
void hashmap<K, V>::clear() {
    _size = 0;
    for (int i = 0; i < _capacity; i++)
        table[i].clear();
}

template <typename K, typename V>
V& hashmap<K, V>::operator[](const K& key) const {
    int index = hash(key) % _capacity;
    for (int i = 0; i < table[index].size(); i++) {
        if (table[index][i].first == key)
            return table[index][i].second;
    }
    return default_value;
}

template <typename K, typename V>
int hashmap<K, V>::hash(const int& key) const {
    int hash         = hash_prime;
    int size_bytes   = sizeof(key);
    const char* data = reinterpret_cast<const char*>(&key);
    for (int i = 0; i < size_bytes; i++)
        hash = (hash << 5) + hash + data[i];
    return hash;
}

template <typename K, typename V>
int hashmap<K, V>::hash(const double& key) const {
    int hash         = hash_prime;
    int size_bytes   = sizeof(key);
    const char* data = reinterpret_cast<const char*>(&key);
    for (int i = 0; i < size_bytes; i++)
        hash = (hash << 5) + hash + data[i];
    return hash;
}

template <typename K, typename V>
int hashmap<K, V>::hash(const string& key) const {
    int hash = hash_prime;
    for (int i = 0; i < key.size(); i++)
        hash = (hash << 5) + hash + key[i];
    return hash;
}

template <typename K, typename V>
void hashmap<K, V>::rehash_table() {
    int new_capacity              = _capacity * 2;
    vector<pair<K, V>>* new_table = new vector<pair<K, V>>[new_capacity];
    for (int i = 0; i < new_capacity; i++)
        new_table[i] = vector<pair<K, V>>();

    for (int i = 0; i < _capacity; i++) {
        for (int j = 0; j < table[i].size(); j++) {
            int new_index = hash(table[i][j].first) % new_capacity;
            new_table[new_index].push_back(table[i][j]);
        }
    }
    // delete[] table;    // need to overload delete[] to not call deconstructors of array object, std implementation has unwanted side effects
    table     = new_table;
    _capacity = new_capacity;
}

#endif