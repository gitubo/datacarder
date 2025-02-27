#pragma once

#include <iostream>
#include <luajit-2.1/lua.hpp>
#include "ChainNode.hpp"

namespace opencmd {

    class NodeFunctionLua : public ChainNode {
    private:
        std::string luaCode;
        mutable lua_State *L;
        int bitstream_to_json_func_ref = LUA_REFNIL;
        int json_to_bitstream_func_ref = LUA_REFNIL;

        static int lua_remove_json_value(lua_State* L) {
            auto json = *static_cast<nlohmann::ordered_json**>(luaL_checkudata(L, 1, "JsonData"));
            const char* key = luaL_checkstring(L, 2);
            if (!json->contains(key)) {
                lua_pushnil(L);  
                return 1;
            }
            (*json).erase(key);
            return 1;
        }

        static int lua_get_json_value(lua_State* L) {
            auto json = *static_cast<nlohmann::ordered_json**>(luaL_checkudata(L, 1, "JsonData"));
            const char* key = luaL_checkstring(L, 2);

            if (!json->contains(key)) {
                lua_pushnil(L);  
                return 1;
            }

            const auto& value = (*json)[key];

            if (value.is_number_integer()) {
                lua_pushinteger(L, value.get<lua_Integer>());  // Restituisce un intero
            } else if (value.is_number_float()) {
                lua_pushnumber(L, value.get<lua_Number>());  // Restituisce un float
            } else if (value.is_string()) {
                lua_pushstring(L, value.get<std::string>().c_str());  
            } else if (value.is_boolean()) {
                lua_pushboolean(L, value.get<bool>()); 
            } else if (value.is_null()) {
                lua_pushnil(L);  
            } else {
                lua_pushnil(L); 
            }

            return 1;
        }

        static int lua_set_json_value(lua_State* L) {
            auto json = *static_cast<nlohmann::ordered_json**>(luaL_checkudata(L, 1, "JsonData"));
            const char* key = luaL_checkstring(L, 2);

            int luaType = lua_type(L, 3);  // Controlla il tipo del valore passato

            switch (luaType) {
                case LUA_TNUMBER: {
                    double num = lua_tonumber(L, 3);  
                    if (std::floor(num) == num) {  
                        (*json)[key] = static_cast<int64_t>(num);  // Integer
                    } else {
                        (*json)[key] = num;  // Floating point
                    }
                    break;
                }
                case LUA_TBOOLEAN:
                    (*json)[key] = static_cast<bool>(lua_toboolean(L, 3));
                    break;
                case LUA_TSTRING:
                    (*json)[key] = lua_tostring(L, 3);
                    break;
                case LUA_TNIL:
                    (*json)[key] = nullptr;
                    break;
                default:
                    luaL_error(L, "Unsupported value type for JSON assignment");
                    break;
            }

            return 0;
        }

        static int lua_convert_uint64_to_bits(lua_State* L) {
            uint64_t x = static_cast<uint64_t>(luaL_checkinteger(L, 1));
            size_t n = static_cast<size_t>(luaL_checkinteger(L, 2));

            lua_newtable(L);  // Creiamo una tabella Lua vuota

            for (size_t i = 0; i < n; ++i) {
                int bit = (x >> (n - 1 - i)) & 1;  // Estraggo il bit più significativo
                lua_pushinteger(L, bit);  // Inserisco il bit nello stack
                lua_rawseti(L, -2, i + 1);  // Lo metto nella tabella a indice (Lua è 1-based)
            }

            return 1;  // Restituisco la tabella Lua
        }


        static int lua_convert_bits_to_uint64(lua_State* L) {
            luaL_checktype(L, 1, LUA_TTABLE);  // Verifico che sia una tabella
            size_t n = lua_objlen(L, 1); 

            uint64_t x = 0;
            for (size_t i = 0; i < n; ++i) {
                lua_rawgeti(L, 1, i + 1);  // Prendo il valore dalla tabella (1-based index)
                int bit = luaL_checkinteger(L, -1);  // Converto a intero
                lua_pop(L, 1);  // Rimuovo il valore dallo stack

                if (bit != 0) {  // Se il valore è 1, imposto il bit corrispondente
                    x |= (1ULL << (n - 1 - i));
                }
            }

            lua_pushinteger(L, x);  // Restituisco l'intero
            return 1;
        }


        static void pass_json_and_bitstream_to_lua(lua_State* L, nlohmann::ordered_json* json, BitStream* bitStream) {

            auto jsonUserData = static_cast<nlohmann::ordered_json**>(lua_newuserdata(L, sizeof(nlohmann::ordered_json*)));
            *jsonUserData = json;
            luaL_getmetatable(L, "JsonData");
            lua_setmetatable(L, -2);


            auto bitStreamUserData = static_cast<BitStream**>(lua_newuserdata(L, sizeof(BitStream*)));
            *bitStreamUserData = bitStream;
            luaL_getmetatable(L, "BitStreamData");
            lua_setmetatable(L, -2);
        }

        void register_global_functions() {
            lua_pushcfunction(L, lua_convert_uint64_to_bits);
            lua_setglobal(L, "uint64_to_bits");

            lua_pushcfunction(L, lua_convert_bits_to_uint64);
            lua_setglobal(L, "bits_to_uint64");
        }


        void register_bitstream_functions() {
            luaL_newmetatable(L, "BitStreamData");

            lua_pushstring(L, "__index");
            lua_newtable(L);

            lua_settable(L, -3);

            lua_pop(L, 1);
        }

        void register_json_functions() {
            luaL_newmetatable(L, "JsonData");

            lua_pushstring(L, "__index"); // Quando json:get() viene chiamato, usa il metatable
            lua_newtable(L);
            lua_pushcfunction(L, lua_get_json_value);
            lua_setfield(L, -2, "get");

            lua_pushcfunction(L, lua_set_json_value);
            lua_setfield(L, -2, "set");

            lua_pushcfunction(L, lua_remove_json_value);
            lua_setfield(L, -2, "erase");

            lua_settable(L, -3); // Assegna la tabella al metatable

            lua_pop(L, 1); // Rimuove la metatabella dallo stack
        }

    public:
        NodeFunctionLua() : ChainNode(), luaCode(""), L(nullptr), bitstream_to_json_func_ref(LUA_REFNIL), json_to_bitstream_func_ref(LUA_REFNIL) {
            L = luaL_newstate();
            luaL_openlibs(L);
            luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);

            register_global_functions();
            register_bitstream_functions();
            register_json_functions();
        }

        ~NodeFunctionLua() {
            if (L) {
                luaL_unref(L, LUA_REGISTRYINDEX, bitstream_to_json_func_ref);
                luaL_unref(L, LUA_REGISTRYINDEX, json_to_bitstream_func_ref);
                lua_close(L);
                L = nullptr;
            }
        }

        void addAttribute(const std::string& key, const ChainNodeAttribute& attribute) override {
            ChainNode::addAttribute(key, attribute);
            if (key == "code" && attribute.isString()) {
                luaCode = attribute.getString().value();

                if (luaL_dostring(L, luaCode.c_str()) != LUA_OK) {
                    Logger::getInstance().error("Error loading Lua code: " + std::string(lua_tostring(L, -1)));
                    lua_pop(L, 1);
                    return;
                }

                lua_getglobal(L, "bitstream_to_json");
                if (!lua_isfunction(L, -1)) {
                    Logger::getInstance().warning("The provided lua code does not contain 'bitstream_to_json' function");
                    lua_pop(L, 1);
                } else {
                    luaJIT_setmode(L, -1, LUAJIT_MODE_FUNC | LUAJIT_MODE_ON);
                    bitstream_to_json_func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
                }

                lua_getglobal(L, "json_to_bitstream");
                if (!lua_isfunction(L, -1)) {
                    Logger::getInstance().warning("The provided lua code does not contain 'json_to_bitstream' function");
                    lua_pop(L, 1);
                } else {
                    luaJIT_setmode(L, -1, LUAJIT_MODE_FUNC | LUAJIT_MODE_ON);
                    json_to_bitstream_func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
                }
            }
        }

        int bitstream_to_json(BitStream& bitStream, nlohmann::ordered_json& outputJson) const override {
            if (!outputJson.is_null() && outputJson.is_object() && bitstream_to_json_func_ref != LUA_REFNIL) {
                lua_rawgeti(L, LUA_REGISTRYINDEX, bitstream_to_json_func_ref);
                pass_json_and_bitstream_to_lua(L, &outputJson, &bitStream);
                if (lua_pcall(L, 2, 0, 0) != LUA_OK) { 
                    Logger::getInstance().error("Error executing bitstream_to_json: " + std::string(lua_tostring(L, -1)));
                    lua_pop(L, 1);
                    return 101;
                }
            }

            if (auto nextNode = getNextNode()) {
                nextNode->bitstream_to_json(bitStream, outputJson);
            }
            return 0;
        }

        int json_to_bitstream(nlohmann::ordered_json& inputJson, BitStream& bitStream) override {
            if (!inputJson.is_null() && inputJson.is_object() && json_to_bitstream_func_ref != LUA_REFNIL) {
                lua_rawgeti(L, LUA_REGISTRYINDEX, json_to_bitstream_func_ref);
                pass_json_and_bitstream_to_lua(L, &inputJson, &bitStream);
                if (lua_pcall(L, 2, 0, 0) != LUA_OK) { 
                    Logger::getInstance().error("Error executing json_to_bitstream: " + std::string(lua_tostring(L, -1)));
                    lua_pop(L, 1);
                    return 101;
                }
            }

            if (auto nextNode = getNextNode()) {
                nextNode->json_to_bitstream(inputJson, bitStream);
            }
            return 0;
        }


    };
}
