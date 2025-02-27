#include "../../include/bitstream/BitStream.hpp"

using namespace opencmd;

BitStream::BitStream(const uint8_t* inputBuffer, size_t initialCapacityInBits)
    : capacity(initialCapacityInBits), offset(0) {

    /* A generic array of unsigned 8bits int is converted 
     * into a uint8_t[] valid for BitStream object
     * Here a couple of examples:
     * 
     * The input is an integer (20) defined by 11 valid bits
     * int 20 => .....000 00010100
     * (bytes)   [   1  ] [   0  ]
     * Converted BitStream
     * buffer => 00000010 100.....
     *           [   0  ] [   1  ]
     * 
     * The input is a string ('hello') defined by 40 valid bits
     * 'hello' => 01101000 01100101 01101100 01101100 01101111
     * (bytes)    [   0  ] [   1  ] [   2  ] [   3  ] [   4  ]
     * Converted BitStream
     * BitStream  01101000 01100101 01101100 01101100 01101111
     *            [   0  ] [   1  ] [   2  ] [   3  ] [   4  ]
     */

    // Check for valid input
    if (!inputBuffer || capacity == 0) {
        throw std::invalid_argument("BitStream::set - Input data is invalid or empty");
    }

    size_t byteCapacity = (capacity + 7) / 8;
    buffer = std::make_unique<uint8_t[]>(byteCapacity);
    size_t bitShiftAmount = capacity % 8;

    if(bitShiftAmount == 0){
        memcpy(buffer.get(), inputBuffer, byteCapacity);
    } else {
        // Circular Right Shift to align to the BitStream approach
        uint8_t* tempBuffer = (uint8_t*) malloc(sizeof(uint8_t)*byteCapacity);
        if(!tempBuffer){
            throw std::invalid_argument("BitStream::set - Impossible to allocate internal buffer"); 
        }

        size_t bitMask = (static_cast<size_t>(1) << bitShiftAmount) - 1;
        uint8_t carry = inputBuffer[0] & bitMask;
        for(int byteIndex = byteCapacity-1; byteIndex >= 0; byteIndex--){
            uint8_t currentByte = (inputBuffer[byteIndex] >> bitShiftAmount) | (carry << 8-bitShiftAmount);
            carry = inputBuffer[byteIndex] & bitMask;
            tempBuffer[byteIndex] = currentByte;
        }

        memcpy(buffer.get(), tempBuffer, byteCapacity);
        free(tempBuffer);
    }
}

BitStream::BitStream(const std::string& base64_str) : offset(0) {
    if (base64_str.empty()) {
        throw std::invalid_argument("BitStream::set - Input base64 string is invalid or empty");
    }
    size_t lengthInBytes = BitStream::base64_decode(base64_str, buffer);
    capacity = lengthInBytes * 8;
}

void BitStream::copyBits(uint8_t* dest, size_t destBitOffset, const uint8_t* src, size_t srcBitOffset, size_t length) const {
    for (size_t i = 0; i < length; ++i) {
        size_t srcByteIndex = (srcBitOffset + i) / 8;
        size_t srcBitIndex = 7 - ((srcBitOffset + i) % 8);

        bool bit = (src[srcByteIndex] >> srcBitIndex) & 1;

        size_t destByteIndex = (destBitOffset + i) / 8;
        size_t destBitIndex = 7 - ((destBitOffset + i) % 8);

        if (bit) {
            dest[destByteIndex] |= (1 << destBitIndex);
        } else {
            dest[destByteIndex] &= ~(1 << destBitIndex);
        }
    }
}

void BitStream::set(const uint8_t* inputBuffer, size_t inputBitLength) {
    if (!inputBuffer || inputBitLength == 0) {
        throw std::invalid_argument("BitStream::set - Input data is invalid or empty");
    }
    capacity = inputBitLength;
    size_t byteCapacity = (capacity + 7) >> 3;            
    buffer = std::make_unique<uint8_t[]>(byteCapacity);
    std::memcpy(buffer.get(), inputBuffer, byteCapacity);
    offset = 0;
}

void BitStream::reduceCapacity(const size_t newCapacity) {
    if(newCapacity>=capacity){
        return;
    }
    size_t byteCapacity = (capacity + 7) / 8;
    size_t newByteCapacity = (newCapacity + 7) / 8;
    std::unique_ptr<uint8_t[]> tempBuffer = std::make_unique<uint8_t[]>(newByteCapacity);
    int byteIndex = byteCapacity-1;
    for(int newByteIndex = newByteCapacity-1; newByteIndex>=0; newByteIndex--){
        tempBuffer[newByteIndex] = buffer[byteIndex--];
    }
    buffer = std::move(tempBuffer);
    capacity=newCapacity;
}
        
std::unique_ptr<uint8_t[]> BitStream::read(size_t length) const {
    if (offset + length > capacity) {
        throw std::out_of_range("BitStream: you are trying to read beyond the end of the bit stream (offset + length > capacity)");
    }

    size_t byteLength = (length + 7) >> 3;
    auto result = std::make_unique<uint8_t[]>(byteLength);
    std::fill(result.get(), result.get() + byteLength, 0);
    copyBits(result.get(), 0, buffer.get(), offset, length);
    
    return result; 
}

std::unique_ptr<uint8_t[]> BitStream::consume(size_t length) {
    if (offset + length > capacity) {
        throw std::out_of_range("BitStream: tentativo di consumare oltre la fine del buffer");
    }
    auto result = read(length); 
    offset += length;
    return result;
}

void BitStream::append(const BitStream& other) {

    size_t newCapacity = capacity + other.capacity;
    size_t newByteCapacity = (newCapacity + 7) / 8;

    std::unique_ptr<uint8_t[]> newBuffer = std::make_unique<uint8_t[]>(newByteCapacity);
    std::fill(newBuffer.get(), newBuffer.get() + newByteCapacity, 0);

    size_t currentByteCapacity = (capacity + 7) / 8;
    std::memcpy(newBuffer.get(), buffer.get(), currentByteCapacity);

    copyBits(newBuffer.get(), capacity, other.buffer.get(), 0, other.capacity);

    buffer = std::move(newBuffer);
    capacity = newCapacity; 
}


int BitStream::shift(const size_t shiftAmount, const bool shiftRight) {
    if (shiftAmount == 0 || capacity == 0){
        return 1;
    }

    if (shiftAmount > capacity) {
        // shifting more bits than the actual capacity 
        std::fill(buffer.get(), buffer.get()+capacity, 0);
        return 0;
    }

    size_t capacityInBytes = (capacity + 7) / 8;
    std::unique_ptr<uint8_t[]> tempBuffer = std::make_unique<uint8_t[]>(capacityInBytes);
    std::fill(tempBuffer.get(), tempBuffer.get() + capacityInBytes, 0);

    if (shiftRight) {
        // Right Shift
/* TBD */
/*
        size_t shiftByte = (shiftAmount+7) / 8;
        size_t shiftBit  = shiftAmount % 8;

        for(int byteIndex = capacityInBytes-1; byteIndex>=0; byteIndex--){

            if(byteIndex+shiftByte > capacityInBytes) continue;

            uint16_t aux16 = buffer[byteIndex];
            aux16 <<= shiftBit;
            uint8_t carry = (aux16 >> 8) & 0xFF;
            uint8_t value = aux16 & 0xFF;

            tempBuffer[byteIndex+shiftByte] |= carry;
            tempBuffer[byteIndex+shiftByte-1] = value;
        }
*/
    } else {
        // Left Shift
        size_t shiftByte = (shiftAmount+7) / 8;
        size_t shiftBit  = shiftAmount % 8;

        for(int byteIndex = capacityInBytes-1; byteIndex>=0; byteIndex--){

            if(byteIndex+shiftByte > capacityInBytes) continue;

            uint16_t aux16 = buffer[byteIndex];
            aux16 <<= shiftBit;
            uint8_t carry = (aux16 >> 8) & 0xFF;
            uint8_t value = aux16 & 0xFF;

            tempBuffer[byteIndex+shiftByte] |= carry;
            tempBuffer[byteIndex+shiftByte-1] = value;
        }
    }
    std::memcpy(buffer.get(), tempBuffer.get(), capacityInBytes); 
    return 0;
}

std::string BitStream::to_string() const {
    return to_string(buffer, capacity);
}

std::string BitStream::to_string(const std::unique_ptr<uint8_t[]>& stream, size_t lengthInBits) {
    std::ostringstream oss;
    int lengthInBytes = (lengthInBits + 7) / 8;
    int bitsInLastByte = 8-(lengthInBits % 8);
    int minBit = 0;
    for(int index=0; index < lengthInBytes; index++){
        if(index==lengthInBytes-1){
            minBit = bitsInLastByte;
        }
        for (int bit = 7; bit >= minBit; bit--) {
            oss << ((stream[index] >> bit) & 1);
        }
    }
    return oss.str();
}

size_t BitStream::base64_decode(const std::string& encoded_string, std::unique_ptr<uint8_t[]>& decoded_data) {
    size_t input_length = encoded_string.length();
    if (input_length % 4 != 0) {
        throw std::invalid_argument("Invalid Base64 string length.");
    }

    size_t padding = 0;
    if (input_length > 0 && encoded_string[input_length - 1] == '=') padding++;
    if (input_length > 1 && encoded_string[input_length - 2] == '=') padding++;
    size_t output_length = (input_length / 4) * 3 - padding;

    decoded_data = std::make_unique<uint8_t[]>(output_length);

    size_t output_index = 0;
    uint32_t buffer = 0;
    int bits_collected = 0;

    for (char c : encoded_string) {
        if (c == '=') break; // Il padding non contribuisce ai dati decodificati
        uint8_t value = base64_lookup[static_cast<uint8_t>(c)];
        if (value == INVALID_CHAR) {
            throw std::invalid_argument("Invalid character in Base64 string.");
        }
        buffer = (buffer << 6) | value; // Aggiunge 6 bit al buffer
        bits_collected += 6;
        if (bits_collected >= 8) {
            bits_collected -= 8;
            decoded_data[output_index++] = (buffer >> bits_collected) & 0xFF;
        }
    }
    if (output_index != output_length) {
        throw std::runtime_error("Decoding error: output length mismatch.");
    }
    return output_length;
};

std::string BitStream::base64_encode(const uint8_t* data, size_t input_length) {
    std::string encoded_string;
    encoded_string.reserve(((input_length + 2) / 3) * 4); 
    size_t i = 0;
    uint32_t buffer = 0;

    while (i < input_length) {
        buffer = data[i++] << 16; 
        if (i < input_length) buffer |= data[i++] << 8; 
        if (i < input_length) buffer |= data[i++];     

        encoded_string.push_back(base64_chars[(buffer >> 18) & 0x3F]);
        encoded_string.push_back(base64_chars[(buffer >> 12) & 0x3F]);
        if ((i - 2) < input_length) {
            encoded_string.push_back(base64_chars[(buffer >> 6) & 0x3F]);
        } else {
            encoded_string.push_back('='); // Padding
        }
        if ((i - 1) < input_length) {
            encoded_string.push_back(base64_chars[buffer & 0x3F]);
        } else {
            encoded_string.push_back('='); // Padding
        }
    }

    return encoded_string;
};