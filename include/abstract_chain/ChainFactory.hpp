#pragma once

#include <iostream>

#include "ChainNode.hpp"

namespace opencmd {

    class ChainFactory{

    public:

        static ChainFactory& getInstance() {
            static ChainFactory instance;
            return instance;
        }


        template <typename T>
        void registerClass(const std::string& className) {
            creators[className] = []() -> std::unique_ptr<ChainNode> {
                return std::make_unique<T>();
            };
        }

        std::unique_ptr<ChainNode> create(const std::string& className) {
            if (creators.find(className) != creators.end()) {
                return creators[className]();
            }
            Logger::getInstance().log("Failed to create class for classname <" + className + ">", Logger::Level::ERROR);
            return nullptr;
            
        }

    private:
        std::unordered_map<std::string, std::function<std::unique_ptr<ChainNode>()>> creators;

    private:
        ChainFactory() = default;
        ChainFactory& operator=(const ChainFactory&) = delete;

    };
}

