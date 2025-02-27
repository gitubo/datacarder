#pragma once

#include <iostream>
#include <lua.hpp>
/*
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>
#include <luajit-2.1/luajit.h>
*/
#include "ChainNode.hpp"

namespace opencmd {
    class NodeFunctionLua : public ChainNode {

    private:
        std::string luaCode;
        mutable lua_State *L;
        int bitstream_to_json_func_ref;
        int json_to_bitstream_func_ref;

        void printLuaStack(lua_State* L) const {
            int top = lua_gettop(L); // Ottiene il numero di elementi nello stack
            std::cout << "Lua Stack (size: " << top << "):\n";

            for (int i = 1; i <= top; i++) { // Scansiona gli elementi nello stack
                int t = lua_type(L, i);

                std::cout << i << ": ";
                switch (t) {
                    case LUA_TSTRING:
                        std::cout << "String: " << lua_tostring(L, i);
                        break;
                    case LUA_TBOOLEAN:
                        std::cout << "Boolean: " << (lua_toboolean(L, i) ? "true" : "false");
                        break;
                    case LUA_TNUMBER:
                        std::cout << "Number: " << lua_tonumber(L, i);
                        break;
                    case LUA_TFUNCTION:
                        std::cout << "Function";
                        break;
                    case LUA_TTABLE:
                        std::cout << "Table";
                        break;
                    case LUA_TNIL:
                        std::cout << "Nil";
                        break;
                    default:
                        std::cout << "Other: " << lua_typename(L, t);
                        break;
                }
                std::cout << std::endl;
            }
        }

        void json_to_lua(lua_State* L, const nlohmann::ordered_json& j) const {
            if (j.is_object()) {
                lua_newtable(L); // Crea una nuova tabella Lua
                for (auto& [key, value] : j.items()) {
                    lua_pushstring(L, key.c_str()); // Chiave
                    json_to_lua(L, value);          // Valore ricorsivo
                    lua_settable(L, -3);            // Imposta nella tabella Lua
                }
            } else if (j.is_array()) {
                lua_newtable(L);
                int index = 1; // Lua usa indici basati su 1
                for (auto& value : j) {
                    //lua_pushinteger(L, index++);
                    json_to_lua(L, value);
                    //lua_settable(L, -3);
                    lua_rawseti(L, -2, index++); 
                }
            } else if (j.is_string()) {
                lua_pushstring(L, j.get<std::string>().c_str());
            } else if (j.is_number_integer()) {
                lua_pushinteger(L, j.get<int>());
            } else if (j.is_number_float()) {
                lua_pushnumber(L, j.get<double>());
            } else if (j.is_boolean()) {
                lua_pushboolean(L, j.get<bool>());
            } else {
                lua_pushnil(L);
            }
        }

        nlohmann::ordered_json lua_to_json(lua_State* L, int index_original) const {
            int index = index_original;
            nlohmann::ordered_json j;

            if (lua_istable(L, index)) {
                bool isArray = true;
                int array_length = 0;
                lua_pushnil(L);  
                while (lua_next(L, index) != 0) {
                    if (!lua_tointeger(L, -2)) {
                        isArray = false;  
                    } else {
                        int key = lua_tointeger(L, -2);
                        if (key != array_length + 1) {
                            isArray = false; 
                        }
                        array_length++;
                    }
                    lua_pop(L, 1);
                }

                if (isArray && array_length > 0) {
                    j = nlohmann::ordered_json::array();

                    for (int i = 1; i <= array_length; ++i) {
                        lua_geti(L, index, i);
                        int t = lua_type(L, -1);
                        switch (t) {
                            case LUA_TSTRING:
                                j.push_back(lua_tostring(L, -1)); break;
                            case LUA_TBOOLEAN:
                                j.push_back(lua_toboolean(L, -1)); break;
                            case LUA_TNUMBER:
                                {
                                    auto num = lua_tonumber(L, -1);
                                    if (num == static_cast<int>(num)) {
                                        j.push_back(static_cast<int>(num));
                                    } else {
                                        j.push_back(num);
                                    }
                                }
                                break;
                            case LUA_TTABLE:
                                j.push_back(lua_to_json(L, lua_gettop(L))); break;
                            case LUA_TNIL:
                                j.push_back(nullptr); break;
                        }

                        lua_pop(L, 1);
                    }
                    return j;
                }

                lua_pushnil(L);  
                while (lua_next(L, index) != 0) { 
                    if (!lua_isstring(L, -2)) {
                        lua_pop(L, 1);
                        continue;
                    }

                    std::string key = lua_tostring(L, -2);
                    int t = lua_type(L, -1);
                    switch (t) {
                        case LUA_TSTRING:
                            j[key] = lua_tostring(L, -1);
                            break;
                        case LUA_TBOOLEAN:
                            j[key] = lua_toboolean(L, -1);
                            break;
                        case LUA_TNUMBER:
                            {
                                auto num = lua_tonumber(L, -1);
                                if (num == static_cast<int>(num)) {
                                    j[key] = static_cast<int>(num);  // Salva come intero
                                } else {
                                    j[key] = num;  // Salva come float
                                }
                            }
                            break;
                        case LUA_TTABLE:
                            j[key] = lua_to_json(L, lua_gettop(L));
                            break;
                        case LUA_TNIL:
                            j[key] = nullptr;
                            break;
                        default:
                            std::cout << "Other: " << lua_typename(L, t);
                            break;
                    }

                    lua_pop(L, 1);
                }
            }
            return j;
        }

    public:
        NodeFunctionLua() : ChainNode(), luaCode("") {}

        ~NodeFunctionLua() {
            luaL_unref(L, LUA_REGISTRYINDEX, bitstream_to_json_func_ref);  
            luaL_unref(L, LUA_REGISTRYINDEX, json_to_bitstream_func_ref);  
            lua_close(L);
        }

        void addAttribute(const std::string& key, const ChainNodeAttribute& attribute) override {
            ChainNode::addAttribute(key,attribute);   
            if(key=="code"){
                if(!attribute.isString()){
                    Logger::getInstance().error("Attribute <lua_code> is not a string");
                    return;
                }
                luaCode = attribute.getString().value();

                L = luaL_newstate();
                luaL_openlibs(L);
                //luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);
                if (luaL_dostring(L, luaCode.c_str()) != LUA_OK) {
                    Logger::getInstance().error(std::string("Error loading Lua code: ") + lua_tostring(L, -1));
                    lua_close(L);
                    return;
                }
                lua_getglobal(L, "bitstream_to_json");
                if (!lua_isfunction(L, -1)) {
                    lua_pop(L, 1);
                    Logger::getInstance().error(std::string("Error loading Lua code: ") + lua_tostring(L, -1));
                    lua_close(L);
                    return;
                }
                //luaJIT_setmode(L, -1, LUAJIT_MODE_FUNC | LUAJIT_MODE_ON);
                bitstream_to_json_func_ref = luaL_ref(L, LUA_REGISTRYINDEX);

                lua_getglobal(L, "json_to_bitstream");
                if (!lua_isfunction(L, -1)) {
                    lua_pop(L, 1);
                    Logger::getInstance().error(std::string("Error loading Lua code: ") + lua_tostring(L, -1));
                    lua_close(L);
                    return;
                }
                //luaJIT_setmode(L, -1, LUAJIT_MODE_FUNC | LUAJIT_MODE_ON);
                json_to_bitstream_func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
            }
        }

        int bitstream_to_json(BitStream& bitStream, nlohmann::ordered_json& outputJson) const override {

//            lua_rawgeti(L, LUA_REGISTRYINDEX, bitstream_to_json_func_ref);
            lua_rawgeti(L, LUA_REGISTRYINDEX, bitstream_to_json_func_ref);  
            if (!lua_isfunction(L, -1)) {
                std::cerr << "Errore: L'elemento recuperato dal registro non è una funzione!" << std::endl;
            }
            lua_pushstring(L, this->getName().c_str());
            lua_pushstring(L, this->getParentName().c_str());
            lua_pushstring(L, "1010101010");
            nlohmann::ordered_json tempJson;
            try{
                tempJson = outputJson.unflatten();
            } catch (const nlohmann::json::exception& e) {
                tempJson = nlohmann::ordered_json::object();;
            }
            json_to_lua(L, tempJson);
 
            if (lua_pcall(L, 4, 1, 0) != LUA_OK) {
                Logger::getInstance().error(std::string("Error executing Lua code: ") + lua_tostring(L, -1));
                lua_close(L);
                return 101;
            }

            int top_index = lua_gettop(L);

            if (!lua_istable(L, top_index)) {
                Logger::getInstance().error("The result of Lua function is not a table");
                lua_pop(L, 1);
                //lua_close(L);
                return 102;
            }
            nlohmann::ordered_json resultParsedAsJson = lua_to_json(L, top_index);

            lua_pop(L, 1);
            //lua_close(L);            

            // Update the json output
            outputJson = resultParsedAsJson.flatten();

            // Propagate to the next node (if any)
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->bitstream_to_json(bitStream, outputJson);
            }

            return 0;
        }

        int json_to_bitstream(nlohmann::ordered_json& inputJson, BitStream& bitStream) override { 
            
//            lua_geti(L, LUA_REGISTRYINDEX, json_to_bitstream_func_ref);
            lua_rawgeti(L, LUA_REGISTRYINDEX, json_to_bitstream_func_ref);  
            if (!lua_isfunction(L, -1)) {
                std::cerr << "Errore: L'elemento recuperato dal registro non è una funzione!" << std::endl;
            }
            lua_pushstring(L, this->getName().c_str());
            lua_pushstring(L, this->getParentName().c_str());
            lua_pushstring(L, "1010101010");
            nlohmann::ordered_json tempJson;
            try{
                tempJson = inputJson.unflatten();
            } catch (const nlohmann::json::exception& e) {
                tempJson = nlohmann::ordered_json::object();;
            }
            json_to_lua(L, tempJson);
 
            if (lua_pcall(L, 4, 1, 0) != LUA_OK) {
                Logger::getInstance().error(std::string("Error executing Lua code: ") + lua_tostring(L, -1));
                //lua_close(L);
                return 101;
            }

            int top_index = lua_gettop(L);

            if (!lua_istable(L, top_index)) {
                Logger::getInstance().error("The result of Lua function is not a table");
                lua_pop(L, 1);
                //lua_close(L);
                return 102;
            }
            nlohmann::ordered_json resultParsedAsJson = lua_to_json(L, top_index);

            lua_pop(L, 1);
            //lua_close(L);            

            // Update the json output
            inputJson = resultParsedAsJson.flatten();

            // Propagate to the next node (if any)
            auto nextNode = getNextNode();
            if(nextNode){
                nextNode->json_to_bitstream(inputJson, bitStream);
            }

            return 0;
        }
    };
}