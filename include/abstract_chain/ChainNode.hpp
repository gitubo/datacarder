#pragma once

#include <string>
#include <variant>
#include <optional>
#include <nlohmann/json.hpp>
#include "../bitstream/BitStream.hpp"
#include "../logger/Logger.hpp"
#include "ChainNodeAttribute.hpp"
#include "ChainAccess.hpp"
#include "Helper.hpp"


namespace opencmd {

    class ChainNode {

    private:
        std::string id;
        std::string name;
        std::string parentName;
        std::unordered_map<std::string, ChainNodeAttribute> attributeMap;
        //std::vector<std::shared_ptr<ChainNode>> children;
        std::shared_ptr<ChainAccess> Chain;
        std::string nextNode;

    public:

        ChainNode() :
            id(""), 
            name(""), 
            parentName("/"), 
            nextNode(""),
            Chain(nullptr) {}
        
        ChainNode(std::string id, std::string name, std::string parentName) : 
            id(id), 
            name(name), 
            parentName(parentName), 
            nextNode(""),
            Chain(nullptr) {}

        // Costruttore di copia
        ChainNode(const ChainNode& other) 
            : id(other.id),
              name(other.name),
              parentName(other.parentName),
              attributeMap(other.attributeMap),
              nextNode(other.nextNode),
              Chain(other.Chain) {
            // Currently the chain is not copied
        }

        ChainNode& operator=(const ChainNode& other) {
            if (this != &other) {
                id = other.id;
                name = other.name;
                parentName = other.parentName;
                attributeMap = other.attributeMap;
                nextNode = other.nextNode;
                Chain = other.Chain;  // Chain is shared, not copied(!)
            }
            return *this;
        }

        virtual std::shared_ptr<ChainNode> clone() const {
            return std::make_shared<ChainNode>(*this);
        }

        virtual ~ChainNode() = default;
        
        const std::string getId() const { return id; }
        const std::string getName() const { return name; }
        const std::string getParentName() const { return parentName; }
        const std::string getFullName() const { return parentName + name;}
        const std::shared_ptr<ChainAccess> getChain() const { return Chain;}
        const std::unordered_map<std::string, ChainNodeAttribute>& getAttributeMap() const { return attributeMap; }
        std::optional<ChainNodeAttribute> getAttribute(const std::string& key) const {
            auto it = attributeMap.find(key);
            if (it != attributeMap.end()) {
                return it->second;
            }
            return std::nullopt;
        }
        
        void setId(std::string id) { this->id = id; }
        void setName(std::string name) { this->name = name; }
        void setParentName(std::string parentName) { 
            if (!parentName.empty() && parentName.back() != '/') {
                parentName += '/';
            }
            this->parentName = parentName; 
        }
        void setNextNode(std::string nextNode) { this->nextNode = nextNode; }
        void setChain(const std::shared_ptr<ChainAccess> Chain) { this->Chain = Chain; }

        virtual void addAttribute(const std::string& key, const ChainNodeAttribute& attribute) {
            attributeMap[key] = attribute;
        }
        void clearAttributes() { attributeMap.clear(); }
/*
        const std::vector<std::shared_ptr<ChainNode>>& getChildren() const { return children; }
        void addChild(const std::shared_ptr<ChainNode>& child) {
            child->setParentName(this->getFullName()); 
            children.push_back(child); 
        }
*/
        virtual std::shared_ptr<ChainNode> getNextNode() const {
            if(!this->nextNode.empty() && this->nextNode!=""){
                return this->Chain->getNode(this->nextNode);
            }
            return nullptr; 
        };

        virtual int bitstream_to_json(BitStream& bitStream, nlohmann::ordered_json& outputJson) const {
            /*
            for (auto& child : this->getChildren()) {
                auto retVal = child->bitstream_to_json(bitStream, outputJson);
            }
            */
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->bitstream_to_json(bitStream, outputJson);
            }
            return 0; 
        };

        virtual int json_to_bitstream(nlohmann::ordered_json& inputJson, BitStream& bitStream) {
            /*
            for (auto& child : this->getChildren()) {
                auto retVal = child->json_to_bitstream(inputJson, bitStream);
            }
            */
            
            // Propagate to the next node (if any)
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->json_to_bitstream(inputJson, bitStream);
            }

            return 0; 
        };

        virtual std::string to_string(size_t indent = 0) const { 
            std::ostringstream oss;
            std::string indentStr(indent, ' ');
            oss << indentStr << "{\n"; 
            oss << indentStr << "  \"id\": \"" << id << "\",\n";
            oss << indentStr << "  \"name\": \"" << name << "\",\n";
            oss << indentStr << "  \"parentName\": \"" << parentName << "\",\n";
            oss << indentStr << "  \"next_node\": \"" << nextNode << "\",\n";
            if(Chain){
                oss << indentStr << "  \"nodes\": \"node_list_reference\",\n";
            } else {
                oss << indentStr << "  \"nodes\": null,\n";
            }
            oss << indentStr << "  \"attributeMap\": {";
            for(auto it = attributeMap.begin(); it != attributeMap.end(); ++it){
                if (it == attributeMap.begin()) { 
                    oss << "\n";
                }
                oss << indentStr << "  \"" << it->first << "\": " << it->second.to_string();
                if (std::next(it) != attributeMap.end()) { 
                    oss << ",\n";
                } else {
                    oss << "\n";
                    oss << indentStr;
                }
            }
            oss << "},\n";
/*
            oss << indentStr << "  \"children\": [";
            for (auto it = children.begin(); it != children.end(); ++it) {
                if (it == children.begin()) {
                    oss << "\n";
                }
                oss << indentStr << (*it)->to_string(indent + 2);
                if (std::next(it) != children.end()) { 
                    oss << ",\n";
                } else {
                    oss << "\n";
                    oss << indentStr;
                }
            }
*/
            oss << "]\n";
            oss << indentStr << "}\n";
            return oss.str();
        }
    };
}

