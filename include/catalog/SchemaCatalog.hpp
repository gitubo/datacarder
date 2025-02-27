#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <optional>
#include <nlohmann/json.hpp>

#include "../logger/Logger.hpp"
#include "../abstract_chain/ChainFactory.hpp"
#include "../abstract_chain/NodeRoot.hpp"
#include "../abstract_chain/NodeDetour.hpp"
#include "../abstract_chain/NodeRouter.hpp"
#include "../abstract_chain/NodeArray.hpp"
#include "../abstract_chain/NodeFunctionLua.hpp"
#include "../abstract_chain/NodeUnsignedInteger.hpp"

#include "Schema.hpp"

namespace opencmd {

    class SchemaCatalog {
    private:
        std::unordered_map<std::string, Schema> schemaMap;

    public:

        static SchemaCatalog& getInstance() {
            static SchemaCatalog instance; 
            return instance;
        }

        int parseSchema(const std::string&, const nlohmann::json&);
        const std::shared_ptr<ChainNode> getAbstractChain(const std::string& key) const;
        std::string to_string(const SchemaElement::SchemaElementArray&);

    private:
        SchemaCatalog() {
            ChainFactory::getInstance().registerClass<NodeRoot>("root");
            ChainFactory::getInstance().registerClass<NodeRoot>("goto");
            ChainFactory::getInstance().registerClass<NodeDetour>("detour");

            ChainFactory::getInstance().registerClass<NodeArray>("array");
            ChainFactory::getInstance().registerClass<NodeRouter>("router");
            ChainFactory::getInstance().registerClass<NodeFunctionLua>("function lua");

            ChainFactory::getInstance().registerClass<NodeUnsignedInteger>("unsigned integer");

        };
        SchemaCatalog& operator=(const SchemaCatalog&) = delete;

        std::optional<std::shared_ptr<ChainNode>> evalArray(const nlohmann::json&, const std::shared_ptr<ChainAccess>);
        std::optional<std::shared_ptr<ChainNode>> evalObject(const nlohmann::json&, const std::shared_ptr<ChainAccess>);
        std::optional<std::shared_ptr<ChainNodeAttribute>> evalAttribute(const nlohmann::json&);
    };

}