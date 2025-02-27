#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>


namespace opencmd {
    class BitStream {
    private:
        std::unique_ptr<uint8_t[]> buffer; 
        size_t capacity;                   
        size_t offset;                     

        void copyBits(uint8_t*, size_t, const uint8_t*, size_t, size_t) const;

    public:

        BitStream() : capacity(0), offset(0), buffer(nullptr) {}
        BitStream(const uint8_t*, size_t);
        BitStream(const std::string&);

        void set(const uint8_t*, size_t);

        std::unique_ptr<uint8_t[]> read(size_t) const;

        std::unique_ptr<uint8_t[]> consume(size_t);

        void append(const BitStream&);

        int shift(const size_t, const bool);

        std::string to_string() const;

        std::string to_base64() const { 
            return base64_encode(this->buffer.get(), (this->capacity + 7) / 8);
        };

        size_t getCapacity() const { return capacity; }
        void reduceCapacity(const size_t);
        size_t getOffset() const { return offset;}
        
    private:

        static std::string to_string(const std::unique_ptr<uint8_t[]>&, size_t);
        
        static constexpr uint8_t INVALID_CHAR = 0xFF;
        static constexpr uint8_t base64_lookup[256] = {
            // 0-31
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR,
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR,
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR,
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR,
            // 32-63
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR,
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, 62, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, 63,  // '+'=62, '/'=63
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR,
            INVALID_CHAR, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  // A-Z
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
            36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  // a-z
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR,
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR,
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR,
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR,
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR,
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR,
            INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR, INVALID_CHAR
        };
        static constexpr char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        static size_t base64_decode(const std::string&, std::unique_ptr<uint8_t[]>&);
        static std::string base64_encode(const uint8_t*, size_t);
    };
}