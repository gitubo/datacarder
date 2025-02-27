#pragma once

#include "ChainNode.hpp"

namespace opencmd {

    class NodeArray : public ChainNode {
    private:
        bool is_array_size_fixed;
        size_t repetitions;
        std::string repetition_reference;
        bool is_absolute_reference;
        std::string arrayNodeId;
        
        virtual std::shared_ptr<ChainNode> getArrayNode() const {
            if(!this->arrayNodeId.empty() && this->arrayNodeId!=""){
                return this->getChain()->getNode(this->arrayNodeId);
            }
            return nullptr; 
        };

    public:
        NodeArray() : ChainNode(), 
            is_array_size_fixed(true),
            repetitions(0),
            repetition_reference(" = "),
            is_absolute_reference(true),
            arrayNodeId("") {}

        NodeArray(const NodeArray& other) : ChainNode(other),
            is_array_size_fixed(other.is_array_size_fixed),
            repetitions(other.repetitions),
            repetition_reference(other.repetition_reference),
            is_absolute_reference(other.is_absolute_reference),
            arrayNodeId(other.arrayNodeId) {}

        NodeArray& operator=(const NodeArray& other) {
            if (this != &other) {
                ChainNode::operator=(other); 
                is_array_size_fixed = other.is_array_size_fixed;
                repetitions = other.repetitions;
                repetition_reference = other.repetition_reference;
                is_absolute_reference = other.is_absolute_reference;
                arrayNodeId = other.arrayNodeId;
            }
            return *this;
        }

        void addAttribute(const std::string& key, const ChainNodeAttribute& attribute) override {
            ChainNode::addAttribute(key,attribute);
            
            if(key=="repetitions"){
                if(attribute.isInteger()) {
                    repetitions = attribute.getInteger().value();
                    is_array_size_fixed = true;
                    repetition_reference = " = ";
                    is_absolute_reference = false;
                } else if(attribute.isString()){
                    repetitions = 0;
                    is_array_size_fixed = false;
                    repetition_reference = attribute.getString().value();
                } else {
                    Logger::getInstance().log("Attribute <repetitions> is not a string or an integer", Logger::Level::ERROR);
                }
            } else if(key=="is_absolute_reference"){
                if(!attribute.isBool()){
                    Logger::getInstance().log("Attribute <is_absolute_path> is not a boolean", Logger::Level::ERROR);
                } else {
                    is_absolute_reference = attribute.getBool().value();
                }
            } else if(key=="array_node_id"){
                if(!attribute.isString()){
                    Logger::getInstance().error("Attribute <array_node_id> is not a string");
                } else {
                    arrayNodeId = attribute.getString().value();
                }
            }
        }

        int bitstream_to_json(BitStream& bitStream, nlohmann::ordered_json& outputJson) const override {
            if(!this->arrayNodeId.empty() && this->arrayNodeId!=""){
                
                // Retrieve the first node of the array structure 
                auto arrayNode = this->getChain()->getNode(this->arrayNodeId);
                if(!arrayNode){
                    Logger::getInstance().error("Missing array node with id <" + this->arrayNodeId + ">");
                    return 1;
                }

                size_t current_repetitions = repetitions;

                // If the size of the array depends on the value of another field
                // of the message, find it and align the repetitions attribute 
                if(!is_array_size_fixed){

                    std::string repetition_reference_key = repetition_reference;
                    
                    // Evaluate the reference in case of not absolute value
                    if(!is_absolute_reference){
                        repetition_reference_key = this->getFullName() + repetition_reference;
                    }

                    // Check if the reference is present
                    if (!outputJson.contains(repetition_reference_key)) {
                        Logger::getInstance().log("Missing repetition reference " + repetition_reference_key + " in the evaluated json", Logger::Level::ERROR);
                        Logger::getInstance().log(outputJson.dump(), Logger::Level::ERROR);
                        return 1;
                    } 

                    // Get the value of the reference
                    auto value = outputJson[repetition_reference_key];
                    
                    // Check the value is a valid integer
                    if (!value.is_number_integer()) {
                        Logger::getInstance().log("Repetition reference value is not an integer", Logger::Level::ERROR);
                        return 2;
                    }
                    current_repetitions = value.get<int>();
                }

                std::string array_basename = std::string(this->getFullName()) + std::string("/"); 
                
                int array_index = 0;
                for(size_t repetition = 0; repetition < current_repetitions; repetition++){
                    // Evaluate the item
                    arrayNode->bitstream_to_json(bitStream, outputJson);
                      
                    // Prepare a local json with the correct key (/array_name/index)
                    // and the value from the evaluated item
                    nlohmann::json j;
                    std::string array_key = array_basename + std::to_string(array_index);
                    j[array_key] = outputJson[arrayNode->getFullName()];
                    
                    // Remove the current item from the actual evaluated json 
                    outputJson.erase(arrayNode->getFullName());

                    // Add the local json to the global json
                    outputJson.update(j);
                    array_index++;     
                }
            } else {
                Logger::getInstance().warning("Array with id <" + this->getId() + "> is missing the array node");
            }

            // Propagate to the next node (if any)
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->bitstream_to_json(bitStream, outputJson);
            }

            return 0;
        };

        int json_to_bitstream(nlohmann::ordered_json& inputJson, BitStream& bitStream) override {
            if(!this->arrayNodeId.empty() && this->arrayNodeId!=""){
                                
                // Retrieve the first node of the array structure 
                auto arrayNode = this->getChain()->getNode(this->arrayNodeId);
                if(!arrayNode){
                    Logger::getInstance().error("Missing array node with id <" + this->arrayNodeId + ">");
                    return 1;
                }

                if(!inputJson.contains(this->getFullName()+"/0")){
                    Logger::getInstance().error("Key <"+this->getFullName()+"/0"+"> not found in the provided json object or the related value is not an array");
                    return 100;      
                }

                int current_repetitions = 0;
                for (auto& [key, val] : inputJson.items()){
                    if(key.rfind(this->getFullName(), 0) == 0){
                        current_repetitions++;
                    }
                }
                int index = 0;
                for(size_t repetition = 0; repetition < current_repetitions; repetition++){
                    /*for (auto& child : this->getChildren()) {
                        child->setName(std::to_string(index));
                        child->setParentName(this->getFullName());
                        child->json_to_bitstream(inputJson, bitStream);
                        index++;
                    }*/
                    auto localArrayNode = arrayNode;
                    localArrayNode->setName(std::to_string(index));
                    localArrayNode->setParentName(this->getFullName());
                    localArrayNode->json_to_bitstream(inputJson, bitStream);
                    index++;
                }
            } else {
                Logger::getInstance().warning("Array with id <" + this->getId() + "> is missing the array node");
            }

            // Propagate to the next node (if any)
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->json_to_bitstream(inputJson, bitStream);
            }

            return 0;
        };


    };
}

