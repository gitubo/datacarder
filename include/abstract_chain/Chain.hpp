#pragma once

#include <string>
#include <variant>
#include <optional>
#include "../logger/Logger.hpp"
#include "ChainAccess.hpp"


namespace opencmd {

    class Chain : public ChainAccess{

    private:
        std::unordered_map<std::string, std::shared_ptr<ChainNode>> chain;
        std::shared_ptr<ChainNode> startingNode;

    public:

        Chain() = default;
        Chain(const Chain& other){
            for(auto it = other.chain.begin(); it != other.chain.end(); ++it){
                chain[it->first] = it->second;
            }     
        }
        Chain& operator=(const Chain& other) {
            if (this != &other) {
                chain.clear();
                for(auto it = other.chain.begin(); it != other.chain.end(); ++it){
                    chain[it->first] = it->second;
                }
            }
            return *this;
        }

        std::unique_ptr<Chain> clone() const { return std::make_unique<Chain>(*this); };

        void addNode(const std::string& nodeId, std::shared_ptr<ChainNode> node) {
            std::pair<std::string, std::shared_ptr<ChainNode>> thisPair(nodeId, node);         
            chain.insert(thisPair);
        }
        void clearBrances() { chain.clear(); }

        void setStartingNode(std::shared_ptr<ChainNode> node) { this->startingNode = node; }
        const std::shared_ptr<ChainNode> getStartingNode() const { return this->startingNode; }

        const std::shared_ptr<ChainNode> getNode(const std::string& name) const override {
            auto it = chain.find(name);
            if (it != chain.end()) {
                return it->second;
            }
            return nullptr;
        }

        int json_to_bitstream(nlohmann::ordered_json& inputJson, BitStream& bitStream){
            auto it = chain.find("main");
            if (it != chain.end()) {
                it->second->json_to_bitstream(inputJson, bitStream);
                return 0;
            } else {
                Logger::getInstance().error("No root node <main>");
            }
            return 1;
        };

        int bitstream_to_json(BitStream& bitStream, nlohmann::ordered_json& outputStream){
            auto it = chain.find("main");
            if (it != chain.end()) {
                it->second->bitstream_to_json(bitStream, outputStream);
                return 0;
            } else {
                Logger::getInstance().error("No root node <main>");
            }
            return 1;
        };

        virtual std::string to_string(size_t indent = 0) const { 
            std::ostringstream oss;
            std::string indentStr(indent, ' ');
            oss << indentStr << "\"nodes\": {";
            for(auto it = chain.begin(); it != chain.end(); ++it){
                if (it == chain.begin()) { 
                    oss << "\n";
                }
                oss << indentStr << "  \"" << it->first << "\": " << it->second->to_string();
                if (std::next(it) != chain.end()) { 
                    oss << ",\n";
                } else {
                    oss << "\n";
                    oss << indentStr;
                }
            }
            oss << indentStr << "}\n";
            return oss.str();
        }
    };
}

