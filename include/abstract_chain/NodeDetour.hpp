#pragma once

#include "ChainNode.hpp"

namespace opencmd {

    class NodeDetour : public ChainNode {
    private:
        std::string detourNodeId = "";
        
        virtual std::shared_ptr<ChainNode> getDetourNode() const {
            if(!this->detourNodeId.empty() && this->detourNodeId!=""){
                return this->getChain()->getNode(this->detourNodeId);
            }
            return nullptr; 
        };

    public:
        NodeDetour() : ChainNode() {}

        void addAttribute(const std::string& key, const ChainNodeAttribute& attribute) override {
            ChainNode::addAttribute(key,attribute);
            if(key=="detour_node_id"){
                if(!attribute.isString()){
                    Logger::getInstance().error("Attribute <detour_node_id> is not a string");
                    return;
                }
                this->detourNodeId = attribute.getString().value();
            }        
        };

        int bitstream_to_json(BitStream& bitStream, nlohmann::ordered_json& outputJson) const override {

            auto detourNode = getDetourNode();
            if(detourNode){
                detourNode->bitstream_to_json(bitStream, outputJson);
            }

            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->bitstream_to_json(bitStream, outputJson);
            }

            return 0; 
        };

        int json_to_bitstream(nlohmann::ordered_json& inputJson, BitStream& bitStream) override {
           
            auto detourNode = getDetourNode();
            if(detourNode){
                detourNode->json_to_bitstream(inputJson, bitStream);
            }
            
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->json_to_bitstream(inputJson, bitStream);
            }
            return 0; 
        };
    };
}

