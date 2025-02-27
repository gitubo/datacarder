#pragma once

#include "ChainNode.hpp"

namespace opencmd {

    class NodeRoot : public ChainNode {

    public:
        NodeRoot() : ChainNode() {}

        void addAttribute(const std::string& key, const ChainNodeAttribute& attribute) override {}
        
        int bitstream_to_json(BitStream& bitStream, nlohmann::ordered_json& outputJson) const override {
            // Propagate to the next node (if any)
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->bitstream_to_json(bitStream, outputJson);
            }

            return 0; 
        };

        int json_to_bitstream(nlohmann::ordered_json& inputJson, BitStream& bitStream) override {
            // Propagate to the next node (if any)
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->json_to_bitstream(inputJson, bitStream);
            }
            return 0; 
        };


    };
}

