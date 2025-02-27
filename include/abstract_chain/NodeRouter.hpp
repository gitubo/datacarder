#pragma once

#include "ChainNode.hpp"

namespace opencmd {

    class NodeRouter : public ChainNode {
    public: 
        enum class Endianness {
            BIG,
            LITTLE
        };
    private:
        size_t bitLength = 0;
        chain_utils::Endianness endianness = chain_utils::Endianness::BIG;
        std::unordered_map<int32_t, std::string> routingTable;

    public:
        NodeRouter() : ChainNode() {}

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
            } else if(key=="routing_table"){
                if(attribute.isArray()) {
                    auto array = attribute.getArray().value();
                    for (auto it = array.begin(); it != array.end(); ++it) {
                        if(!it->isArray()){
                            Logger::getInstance().error("Attribute <routing_table> does not contain arrays");                           
                            return;
                        }
                        auto routing_rule = it->getArray().value();
                        if(routing_rule.size()!=2){
                            Logger::getInstance().error("Each routing rule in <routing_table> must contains 2 elements");  
                            return;                         
                        }
                        if(!routing_rule[0].isInteger() || !routing_rule[1].isString()){
                            Logger::getInstance().error("Each routing rule in <routing_table> must contains an integer as first element and a string as second element");  
                            return;                      
                        }
                        std::pair<int32_t, std::string> thisRule(routing_rule[0].getInteger().value(), routing_rule[1].getString().value());
                        routingTable.insert(thisRule);
                    }
                } else {
                    Logger::getInstance().log("Attribute <routing_table> is not an array", Logger::Level::ERROR);
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
                Logger::getInstance().error("Failed to convert to uint64_t <" + this->getFullName() + ">");
                return 0;
            }

            // Add the value to the output json
            outputJson[this->getFullName()] = value;
            
            // Address the dissector to the right route
            auto it = routingTable.find(value);
            if (it == routingTable.end()) {
                Logger::getInstance().error("Missing routing rule with value <"+std::to_string(value)+">");
                return 3;
            }
            auto chain = getChain()->getNode(routingTable.at(value));
            chain->bitstream_to_json(bitStream, outputJson);
            
            // Propagate to the next node (if any)
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->bitstream_to_json(bitStream, outputJson);
            }

            return 0;
        };

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

            // Address the dissector to the right route
            auto it = routingTable.find(value);
            if (it == routingTable.end()) {
                Logger::getInstance().error("Missing routing rule with value <"+std::to_string(value)+">");
                return 3;
            }
            auto chain = getChain()->getNode(routingTable.at(value));
            chain->json_to_bitstream(inputJson, bitStream);

            // Propagate to the next node (if any)
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->json_to_bitstream(inputJson, bitStream);
            }

            return 0;
        };


    };
}

