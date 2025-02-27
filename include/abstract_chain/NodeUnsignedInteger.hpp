#pragma once

#include "ChainNode.hpp"

namespace opencmd {
    class NodeUnsignedInteger : public ChainNode {

    public:

    private:
        size_t bitLength = 0;
        chain_utils::Endianness endianness = chain_utils::Endianness::BIG;

    public:
        NodeUnsignedInteger() : ChainNode() {}

        void addAttribute(const std::string& key, const ChainNodeAttribute& attribute) override {
            ChainNode::addAttribute(key,attribute);
            
            if(key=="bit_length"){
                if(!attribute.isInteger()){
                    Logger::getInstance().log("Attribute <bit_length> is not an integer", Logger::Level::ERROR);
                } else {
                    bitLength = attribute.getInteger().value();
                    if(bitLength<=0){
                        Logger::getInstance().log("Attribute <bit_length> is not valid (<=0)", Logger::Level::ERROR);
                        bitLength = 0;
                    }
                }
            } else if(key=="endianness"){
                if(!attribute.isString()){
                    Logger::getInstance().log("Attribute <endianness> is not a string", Logger::Level::ERROR);
                } else {
                    auto endianess_str = attribute.getString().value();
                    if(endianess_str.compare("big")){
                        endianness = chain_utils::Endianness::BIG;
                    } else if(endianess_str.compare("little")){
                        endianness = chain_utils::Endianness::LITTLE;
                    } else {
                        endianness = chain_utils::Endianness::BIG;
                        Logger::getInstance().error("Attribute <endianness> is not valid ("+endianess_str+")");
                        Logger::getInstance().warning("Attribute <endianness> forced to 'BIG'");
                    }
                }
            }
        }

        int bitstream_to_json(BitStream& bitStream, nlohmann::ordered_json& outputJson) const override {
            uint64_t value = 0;
            int convertion_error = chain_utils::convert_from_uint8_array_to_uint64(
                bitStream.consume(bitLength).get(),
                bitLength,
                endianness,
                &value);

            if(convertion_error){
                Logger::getInstance().error("Failed to convert to uint64_t [" + this->getFullName() + "]");
                return 0;
            }

            // Prepare the json output
            outputJson[this->getFullName()] = value;

            // Propagate to the next node (if any)
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->bitstream_to_json(bitStream, outputJson);
            }

            return 0;
        }

        int json_to_bitstream(nlohmann::ordered_json& inputJson, BitStream& bitStream) override { 
            if(!inputJson.is_object()){
                Logger::getInstance().error("The provided json is not an object");
                return 100;
            }
            if(!inputJson.contains(this->getFullName()) || !inputJson[this->getFullName()].is_number_integer()){
                Logger::getInstance().error("Key <"+this->getFullName()+"> not found in the provided json object or the related value is not an integer");
                return 100;      
            }
            uint64_t rawValue = inputJson[this->getFullName()];
            uint64_t value = 0;
            switch (endianness){
                case chain_utils::Endianness::BIG:
                    value = rawValue;
/*                     for (int i = 0; i < sizeof(uint64_t); ++i) {
                        value |= ((rawValue >> (i * 8)) & 0xFF) << ((7 - i) * 8);
                    }
 */                    break;
                case chain_utils::Endianness::LITTLE:
                    // No convertion needed
                    value = rawValue;
                    break;
                default:
                    Logger::getInstance().warning("Unsupported endianness");
                    return 100;
            }
     
            uint8_t inputBuffer[(bitLength+7)/8];
            std::memcpy(inputBuffer, &value, (bitLength+7)/8);
            BitStream bsInteger(inputBuffer, bitLength);
            bitStream.append(bsInteger);

            // Propagate to the next node (if any)
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->json_to_bitstream(inputJson, bitStream);
            }

            return 0;
        }
    };
}