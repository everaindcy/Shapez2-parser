#pragma once

#include "shape.hpp"
#include "shape.cpp"

#include <cassert>
#include <vector>
#include <iostream>
#include <string>
#include <queue>
#include <set>
#include <map>
#include <thread>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <cinttypes>

// #define OUTPUT_LOG

const int THREADS = 79;

// const int MAX_HIGHT = 4;
// const int TEST_POINTS = 1000;
// const int TEST_POINTS_SMALL = 5;

const int QUAD_SIZE = 4;
const int MAX_HIGHT = 5;
const int TEST_POINTS = 100000;
const int TEST_POINTS_SMALL = 100;

const int CODE_SHIFT = MAX_HIGHT*QUAD_SIZE*2;
const u64 MAX_INDEX = 0xFFFFFFFFFFFFFFFF >> (64-CODE_SHIFT);
const u64 MAX_QUAD_INDEX = 0xFFFFFFFFFFFFFFFF >> (64-MAX_HIGHT*2);

const u64 PIN_CODE = 0xFF;

inline u64 getIdx(const u64 value) {
    return value & MAX_INDEX;
}
inline u64 getMtd(const u64 value) {
    return value >> CODE_SHIFT;
}
inline u64 CreateValue(u64 idx, u64 mtd) {
    return idx | (mtd << CODE_SHIFT);
}

const u64 MAX_MTD_MAIN = 9;
const std::vector<Shape> stackShapes = {
    Shape("CuCu----", MAX_HIGHT),
    Shape("--CuCu--", MAX_HIGHT),
    Shape("----CuCu", MAX_HIGHT),
    Shape("Cu----Cu", MAX_HIGHT),

    Shape("CuCuCu--", MAX_HIGHT),
    Shape("--CuCuCu", MAX_HIGHT),
    Shape("Cu--CuCu", MAX_HIGHT),
    Shape("CuCu--Cu", MAX_HIGHT),

    Shape("CuCuCuCu", MAX_HIGHT),

    // after do not used in main
    Shape("Cu------", MAX_HIGHT),
    Shape("--Cu----", MAX_HIGHT),
    Shape("----Cu--", MAX_HIGHT),
    Shape("------Cu", MAX_HIGHT),

    Shape("P-------", MAX_HIGHT),
    Shape("--P-----", MAX_HIGHT),
    Shape("----P---", MAX_HIGHT),
    Shape("------P-", MAX_HIGHT),

    // after do not needed
    Shape("Cu--Cu--", MAX_HIGHT),
    Shape("--Cu--Cu", MAX_HIGHT),

    Shape("CuP-----", MAX_HIGHT),
    Shape("--CuP---", MAX_HIGHT),
    Shape("----CuP-", MAX_HIGHT),
    Shape("P-----Cu", MAX_HIGHT),

    Shape("Cu--P---", MAX_HIGHT),
    Shape("--Cu--P-", MAX_HIGHT),
    Shape("P---Cu--", MAX_HIGHT),
    Shape("--P---Cu", MAX_HIGHT),

    Shape("Cu----P-", MAX_HIGHT),
    Shape("P-Cu----", MAX_HIGHT),
    Shape("--P-Cu--", MAX_HIGHT),
    Shape("----P-Cu", MAX_HIGHT),

    Shape("P-P-----", MAX_HIGHT),
    Shape("--P-P---", MAX_HIGHT),
    Shape("----P-P-", MAX_HIGHT),
    Shape("P-----P-", MAX_HIGHT),

    Shape("P---P---", MAX_HIGHT),
    Shape("--P---P-", MAX_HIGHT),

    Shape("CuCuP---", MAX_HIGHT),
    Shape("--CuCuP-", MAX_HIGHT),
    Shape("P---CuCu", MAX_HIGHT),
    Shape("CuP---Cu", MAX_HIGHT),

    Shape("CuCu--P-", MAX_HIGHT),
    Shape("P-CuCu--", MAX_HIGHT),
    Shape("--P-CuCu", MAX_HIGHT),
    Shape("Cu--P-Cu", MAX_HIGHT),

    Shape("CuP-Cu--", MAX_HIGHT),
    Shape("--CuP-Cu", MAX_HIGHT),
    Shape("Cu--CuP-", MAX_HIGHT),
    Shape("P-Cu--Cu", MAX_HIGHT),

    Shape("CuP-P---", MAX_HIGHT),
    Shape("--CuP-P-", MAX_HIGHT),
    Shape("P---CuP-", MAX_HIGHT),
    Shape("P-P---Cu", MAX_HIGHT),

    Shape("CuP---P-", MAX_HIGHT),
    Shape("P-CuP---", MAX_HIGHT),
    Shape("--P-CuP-", MAX_HIGHT),
    Shape("P---P-Cu", MAX_HIGHT),

    Shape("Cu--P-P-", MAX_HIGHT),
    Shape("P-Cu--P-", MAX_HIGHT),
    Shape("P-P-Cu--", MAX_HIGHT),
    Shape("--P-P-Cu", MAX_HIGHT),

    Shape("P-P-P---", MAX_HIGHT),
    Shape("--P-P-P-", MAX_HIGHT),
    Shape("P---P-P-", MAX_HIGHT),
    Shape("P-P---P-", MAX_HIGHT),

    Shape("CuCuCuP-", MAX_HIGHT),
    Shape("P-CuCuCu", MAX_HIGHT),
    Shape("CuP-CuCu", MAX_HIGHT),
    Shape("CuCuP-Cu", MAX_HIGHT),

    Shape("CuCuP-P-", MAX_HIGHT),
    Shape("P-CuCuP-", MAX_HIGHT),
    Shape("P-P-CuCu", MAX_HIGHT),
    Shape("CuP-P-Cu", MAX_HIGHT),

    Shape("CuP-CuP-", MAX_HIGHT),
    Shape("P-CuP-Cu", MAX_HIGHT),

    Shape("CuP-P-P-", MAX_HIGHT),
    Shape("P-CuP-P-", MAX_HIGHT),
    Shape("P-P-CuP-", MAX_HIGHT),
    Shape("P-P-P-Cu", MAX_HIGHT),

    Shape("P-P-P-P-", MAX_HIGHT)
};

inline std::string getTimeStringHMS(std::chrono::duration<double> duration) {
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    duration -= hours;
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= minutes;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    return std::to_string(hours.count()) + "h " + std::to_string(minutes.count()) + "m " + std::to_string(seconds.count()) + "s";
}

inline bool saveMap(const char* outFile, const std::map<u64, u64> &m) {
    auto file = fopen(outFile, "w");
    if (!file) {
        std::cerr << "Error opening " << outFile << " for writing." << std::endl;
        return false;
    }
    for (const auto& [idx, value] : m) {
        fprintf(file, "%" PRIx64 " %" PRIx64 "\n", idx, value);
    }
    fclose(file);
    std::cerr << "Saved " << m.size() << " shapes to " << outFile << "." << std::endl;
    return true;
}

inline bool saveMapBinary(const char* outFile, const std::map<u64, u64> &m) {
    auto file = fopen(outFile, "wb");
    if (!file) {
        std::cerr << "Error opening " << outFile << " for writing." << std::endl;
        return false;
    }
    for (const auto& [idx, value] : m) {
        fwrite(&idx, sizeof(idx), 1, file);
        fwrite(&value, sizeof(value), 1, file);
    }
    fclose(file);
    std::cerr << "Saved " << m.size() << " shapes to " << outFile << "." << std::endl;
    return true;
}

inline bool loadMap(const char* inFile, std::map<u64, u64> &m) {
    auto file = fopen(inFile, "r");
    if (!file) {
        std::cerr << "Error opening " << inFile << " for reading." << std::endl;
        return false;
    }
    u64 idx, value;
    while (fscanf(file, "%" PRIx64 "%" PRIx64, &idx, &value) == 2) {
        m[idx] = value;
    }
    fclose(file);
    std::cerr << "Loaded " << m.size() << " shapes from " << inFile << "." << std::endl;
    return true;
}

inline bool loadMapBinary(const char* inFile, std::map<u64, u64> &m) {
    auto file = fopen(inFile, "rb");
    if (!file) {
        std::cerr << "Error opening " << inFile << " for reading." << std::endl;
        return false;
    }
    u64 idx, value;
    while (fread(&idx, sizeof(idx), 1, file) == 1 && fread(&value, sizeof(value), 1, file) == 1) {
        m[idx] = value;
    }
    fclose(file);
    std::cerr << "Loaded " << m.size() << " shapes from " << inFile << "." << std::endl;
    return true;
}

class fileMap {
    public:
    std::ifstream file;
    u64 size_;
    const u64 itemSize = 2*sizeof(u64); // 1 for index, 1 for method

    // Delete copy constructor and copy assignment operator
    fileMap(const fileMap&) = delete;
    fileMap& operator=(const fileMap&) = delete;

    fileMap(const char* filename) {
        file.open(filename, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            throw std::runtime_error("File open error");
        }
        file.seekg(0, std::ios::end);
        size_ = file.tellg() / itemSize;
        std::cerr << "Loaded " << size_ << " items from " << filename << "." << std::endl;
    }
    ~fileMap() {
        if (file.is_open()) {
            file.close();
        }
    }

    void close() {
        if (file.is_open()) {
            file.close();
        }
    }

    int count(u64 idx) {
        if (cachedIndex == idx) {
            return 1; // Already cached
        }
        u64 left = 0;
        u64 right = size_;
        for (;;) {
            if (left >= right) {
                return 0; // Not found
            }
            u64 now = (left + right) / 2;
            file.seekg(now * itemSize, std::ios::beg);
            u64 index, value;
            file.read(reinterpret_cast<char*>(&index), sizeof(index));
            file.read(reinterpret_cast<char*>(&value), sizeof(value));

            if (index < idx) {
                left = now + 1;
            } else if (index > idx) {
                right = now;
            } else {
                cachedIndex = index;
                cachedValue = value;
                return 1;
            }
        }
    }

    u64 operator[](u64 idx) {
        if (cachedIndex == idx) {
            return cachedValue; // Already cached
        }
        u64 left = 0;
        u64 right = size_;
        for (;;) {
            if (left >= right) {
                return 0; // Not found
            }
            u64 now = (left + right) / 2;
            file.seekg(now * itemSize, std::ios::beg);
            u64 index, value;
            file.read(reinterpret_cast<char*>(&index), sizeof(index));
            file.read(reinterpret_cast<char*>(&value), sizeof(value));

            if (index < idx) {
                left = now + 1;
            } else if (index > idx) {
                right = now;
            } else {
                cachedIndex = index;
                cachedValue = value;
                return value;
            }
        }
    }

    class iterator {
        fileMap* fm;
        u64 index;
        std::pair<u64,u64> value;
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = std::pair<u64,u64>;
        using difference_type = std::ptrdiff_t;
        using pointer = std::pair<u64,u64>*;
        using reference = std::pair<u64,u64>&;

        iterator(fileMap* fm, u64 index, std::pair<u64,u64> value) : fm(fm), index(index), value(value) {}

        bool operator!=(const iterator& other) const {
            return index != other.index;
        }

        bool operator==(const iterator& other) const {
            return index == other.index;
        }

        iterator& operator++() {
            // Move to the next item
            index++;
            if (fm && index < fm->size_) {
                fm->file.seekg(index * fm->itemSize, std::ios::beg);
                fm->file.read(reinterpret_cast<char*>(&value.first), sizeof(value.first));
                fm->file.read(reinterpret_cast<char*>(&value.second), sizeof(value.second));
            } else {
                value = {0, 0}; // End of iteration
            }
            return *this;
        }

        std::pair<u64,u64> operator*() const {
            return value;
        }

        std::pair<u64,u64>* operator->() {
            return &value;
        }
    };
    using const_iterator = iterator;

    const_iterator begin() {
        file.seekg(0, std::ios::beg);
        u64 index, value;
        file.read(reinterpret_cast<char*>(&index), sizeof(index));
        file.read(reinterpret_cast<char*>(&value), sizeof(value));
        return iterator(this, 0, {index, value});
    }

    const_iterator end() {
        return iterator(this, size_, {0, 0});
    }

    u64 size() const {
        return size_;
    }

private:
    u64 cachedIndex;
    u64 cachedValue;
};