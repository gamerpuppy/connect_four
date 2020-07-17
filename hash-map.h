//
// Created by keega on 7/16/2020.
//

#ifndef CONNECT_FOUR_HASH_MAP_H
#define CONNECT_FOUR_HASH_MAP_H

#include <cstdint>
#include <iostream>

static uint64_t tooManySubmaps = 0;
static uint64_t tooManyEntries = 0;

static constexpr uint64_t SUBMAP_CAPACITY[] = {1,8,48,64,128,128,384,512,1024,1024,1024,1024,1024};

template <class T>
class MultiHashMap {
private:
    static const uint64_t PRIME = 18243357615227;
    static const uint64_t EMPTY = (uint64_t)1u << 63u;

    struct Entry {
        uint64_t key;
        T obj;

        Entry(): key(EMPTY) {}
        Entry(uint64_t key, const T &obj): key(key), obj(obj) {}
    };

    struct SubMap {
        uint64_t key;
        uint64_t capacity;
        Entry* entries;
        uint64_t size = 0;

        SubMap(): key(EMPTY), capacity(0), entries(nullptr) {}
        explicit SubMap(uint64_t key, uint64_t capacity): key(key), capacity(capacity), entries(new Entry[capacity]) {
            for (uint64_t i = 0; i < capacity; i++) {
                entries[i] = Entry();
            }
        }

        SubMap(SubMap &&rhs) noexcept {
            key = rhs.key;
            capacity = rhs.capacity;
            entries = rhs.entries;
            size = rhs.size;
            rhs.entries = nullptr;
        }

        SubMap& operator=(SubMap &&rhs) noexcept {
            key = rhs.key;
            capacity = rhs.capacity;
            entries = rhs.entries;
            size = rhs.size;
            rhs.entries = nullptr;
            return *this;
        }

        SubMap(const SubMap &rhs) = delete;
        SubMap& operator=(const SubMap &rhs) = delete;

        ~SubMap() {
            if (entries != nullptr) {
                delete[] entries;
            }
        }

        bool put(uint64_t entryKey, const T &obj) {
            if (size+1 == capacity || (double)size / capacity > 0.85 ) {
                tooManyEntries++;
                return false;
            }
            int idx = ((entryKey % PRIME) + (entryKey >> 32 % PRIME)) % capacity;
            while (true) {
                if (idx >= capacity) {
                    idx = 0;
                }

                if (entries[idx].key == EMPTY) {
                    entries[idx] = Entry(entryKey, obj);
                    size++;
                    return true;

                } else if (entries[idx].key == entryKey) {
//                    entries[idx] = Entry(entryKey, obj);
//                    return true;

                } else {
                    idx++;
                }
            }
        }

        T* get(uint64_t entryKey) {
            int idx = ((entryKey % PRIME) + (entryKey >> 32 % PRIME)) % capacity;
            while (true) {
                if (idx >= capacity) {
                    idx = 0;
                }

                if (entries[idx].key == EMPTY) {
                    return nullptr;

                } else if (entries[idx].key == entryKey) {
                    return &entries[idx].obj;

                } else {
                    idx++;
                }
            }

        }

    };

    SubMap* subMaps;
    const uint64_t capacity;
    uint64_t size = 0;

    SubMap* getOrMakeSubMapForKey(uint64_t subMapKey, uint32_t hashDepth) {
        if (size+1 == capacity || (double)size / capacity > 0.85 ) {
            tooManySubmaps++;
            return nullptr;
        }
        int idx = ((subMapKey % PRIME) + (subMapKey >> 32 % PRIME)) % capacity;
        while (true) {
            if (idx >= capacity) {
                idx = 0;
            }

            if (subMaps[idx].key == EMPTY) {
                subMaps[idx] = SubMap(subMapKey, SUBMAP_CAPACITY[hashDepth]);
                size++;

                SubMap* ptr = &subMaps[idx];
                return ptr;
            } else if (subMaps[idx].key == subMapKey) {
                return &subMaps[idx];
            } else {
                idx++;
            }
        }
    }

    SubMap* getSubMapForKey(uint64_t subMapKey) {
        int idx = ((subMapKey % PRIME) + (subMapKey >> 32 % PRIME)) % capacity;
        while (true) {
            if (idx >= capacity) {
                idx = 0;
            }

            if (subMaps[idx].key == EMPTY) {
                return nullptr;
            } else if (subMaps[idx].key == subMapKey) {
                return &subMaps[idx];
            } else {
                idx++;
            }
        }
    }

public:
    explicit MultiHashMap(int capacity): capacity(capacity), subMaps(new SubMap[capacity]) {
        for (uint64_t i = 0; i < capacity; i++) {
            subMaps[i] = SubMap();
        }
    }
    ~MultiHashMap() {
        delete[] subMaps;
    }

    void printMetrics() {


    }

    bool put(uint64_t key1, uint64_t key2, const T &obj, uint32_t hashDepth) {
        SubMap* subMapPtr = getOrMakeSubMapForKey(key1, hashDepth);
        if (subMapPtr == nullptr) {
            return false;
        }
        subMapPtr->put(key2, obj);
        return true;
    }

    T* get(uint64_t key1, uint64_t key2) {
        // TODO CHANGE
        SubMap* subMapPtr = getSubMapForKey(key1);
        if (subMapPtr == nullptr) {
            return nullptr;
        }
        return subMapPtr->get(key2);
    }

};

#endif //CONNECT_FOUR_HASH_MAP_H
