#pragma once

#include <cstdint> 
#include <algorithm> 
#include <type_traits> 

namespace chain_utils {

    enum class Endianness {
        BIG,
        LITTLE
    };

    constexpr Endianness getSystemEndianness() {
        union {
            uint32_t i;
            char c[4];
        } bint = {0x01020304};

        return (bint.c[0] == 0x01) ? Endianness::BIG : Endianness::LITTLE;
    }

    template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type
    changeEndianness(T value) {
        static_assert(sizeof(T) <= 8, "Type size exceeds 8 bytes.  Unsupported.");

        T result = value;
        unsigned char* bytes = reinterpret_cast<unsigned char*>(&result);
        std::reverse(bytes, bytes + sizeof(T));
        return result;
    }

    inline uint64_t convert_from_uint8_array_to_uint64(
        uint8_t* buffer, 
        size_t bitLength, 
        Endianness endianness,
        uint64_t* result)
    {
        size_t numberOfBytes = bitLength + 7 >> 3;
        switch (endianness){
            case Endianness::BIG:
                for (size_t i = 0; i < numberOfBytes; ++i) {
                    *result |= buffer[i] << ((numberOfBytes - 1 - i) * 8);
                }
                break;
            case Endianness::LITTLE:
                for (size_t i = 0; i < numberOfBytes; ++i) {
                    *result |= buffer[i] << (i * 8);
                }
                break;
            default:
                return 1;
        }
        if (bitLength % 8) {
            *result >>= (8 - (bitLength % 8));
        } 
        return 0;
    }

}