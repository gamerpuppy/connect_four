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
        Entry& operator=(const Entry &rhs) {
            key = rhs.key;
            obj = rhs.obj;
        }
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

template<class T, size_t cap>
class MultiHashMap2 {
private:
    static const uint32_t EMPTY = 0xFFFFFFFFu;
    static const uint64_t PRIME1 = 3426051125257;
    static const uint64_t PRIME2 = 2674691661143;
    static const uint64_t PRIME3 = 1178781740629;

    struct Entry {
        uint32_t keyPart1;
        uint64_t keyPart2;
        T obj;

        static const uint64_t KEY2_MASK = ~0xFFFFull;

        Entry(): keyPart1(EMPTY), keyPart2(0), obj() {}
        Entry(uint64_t key1, uint64_t key2, T obj): keyPart1(key1 >> 16u), keyPart2(key1 << 48u | key2), obj() {}
        Entry(const Entry &rhs): keyPart1(rhs.keyPart1), keyPart2(rhs.keyPart2), obj(rhs.obj) {}

        bool keysEqual(uint64_t key1, uint64_t key2) {
            uint64_t gKey1 = keyPart1 << 32u | keyPart2 >> 48u;
            uint64_t gKey2 = keyPart2 & KEY2_MASK;
            return gKey2 == key2 && gKey1 == key1;
        }

        bool isEmpty() {
            return keyPart1 == EMPTY;
        }

    };

    std::array<Entry, cap> arr;
    uint64_t size = 0;
    const uint64_t  maxSize = cap * 0.85;

    size_t getIdxForKeys(uint64_t key1, uint64_t key2) {
        return (((key1 % PRIME1) + (key2 % PRIME2)) % PRIME3) % cap;
    }

    // assumes duplicates are not added
    bool put(uint64_t key1, uint64_t key2, T obj) {
        if (size >= maxSize) {
            return false;
        }

        size_t idx = getIdxForKeys(key1, key2);
        while (true) {
            if (idx >= cap) {
                idx = 0;
            }

            if (arr[idx].isEmpty()) {
                arr[idx] = Entry(key1, key2, obj);
                size++;
                return true;

            } else {
                idx++;
            }
        }
    }

    T* get(uint64_t key1, uint64_t key2) {
        size_t idx = getIdxForKeys(key1, key2);
        while (true) {
            if (idx >= cap) {
                idx = 0;
            }

            if (arr[idx].isEmpty()) {
               return nullptr;
            } else if (arr[idx].keysEqual(key1, key2)) {
                return &arr[idx].obj;
            } else {
                idx++;
            }
        }
    }

};

#endif //CONNECT_FOUR_HASH_MAP_H
