#pragma once

#include <iostream>
#include <sstream>
#include <fstream>

#include "SchemaElement.hpp"

#include "../logger/Logger.hpp"
#include "../abstract_chain/Chain.hpp"

namespace opencmd {
    class Schema {
    private:
        std::string catalogName;
        std::string version;
        std::unordered_map<std::string, std::string> metadata;
        std::shared_ptr<Chain> chain;

    public:
        Schema() : chain(std::make_shared<Chain>()) {};
        const std::string getCatalogName() const { return this->catalogName; }
        const std::string getVersion() const { return this->version; }
        const std::unordered_map<std::string, std::string> getMetadata() const { return this->metadata; }
        const std::shared_ptr<Chain> getChain() const { return this->chain; }
        
        void setCatalogName(const std::string& name){ this->catalogName = name;}
        void setVersion(const std::string& version){ this->version = version;}
        void setMetadata(const std::unordered_map<std::string, std::string>& metadata){ this->metadata = metadata;}

        void addNode(const std::string& nodeId, std::shared_ptr<ChainNode> nodeStructure) {
            if(!chain){
                Logger::getInstance().error("Null tree pointer: impossible to add node with name <"+nodeId+">");
                return;
            }
            chain->addNode(nodeId, nodeStructure);
        }
        void setChainStartingNode(std::shared_ptr<ChainNode> startingNode){
            chain->setStartingNode(startingNode);
        }

        const std::shared_ptr<ChainNode> getChainStartingNode() const { 
            if(!chain){
                Logger::getInstance().error("Null tree pointer: impossible to get starting node");
                return nullptr;
            }
            return chain->getStartingNode();
        }        

        const std::shared_ptr<ChainNode> getNode(const std::string& nodeName) const { 
            if(!chain){
                Logger::getInstance().error("Null tree pointer: impossible to get node with name <"+nodeName+">");
                return nullptr;
            }
            return chain->getNode(nodeName);
        }        

        std::string to_string(size_t indent = 0) const {
            std::ostringstream oss;
            std::string indentStr(indent, ' ');
            oss << "{\n" << indentStr; 
            oss << indentStr << "  \"catalogName\": \"" << catalogName << "\",\n";
            oss << indentStr << "  \"version\": \"" << version << "\",\n";
            //oss << indentStr << "  \"metadata\": " << std::string(metadata) << ",\n";
            oss << indentStr << "  \"nodes\": " << "..." << "\n";
            oss << "}\n";
            return oss.str();
        }
    };

}