#pragma once

#include <string>
#include <variant>
#include <optional>

namespace opencmd {

class ChainNodeAttribute {

    public:
        using ChainNodeAttributeObject = std::unordered_map<std::string, ChainNodeAttribute>;
        using ChainNodeAttributeArray = std::vector<ChainNodeAttribute>;
        using ChainNodeAttributeVariant = std::variant<
            std::nullptr_t, 
            bool, 
            double, 
            int64_t, 
            std::string, 
            ChainNodeAttributeObject, 
            ChainNodeAttributeArray
        >;

    private:
        ChainNodeAttributeVariant value;

    public:
        ChainNodeAttribute() : value(nullptr) {}
        ChainNodeAttribute(bool b) : value(b) {}
        ChainNodeAttribute(double d) : value(d) {}
        ChainNodeAttribute(int64_t i) : value(i) {}
        ChainNodeAttribute(const std::string& s) : value(s) {}
        ChainNodeAttribute(const ChainNodeAttributeObject& o) : value(o) {}
        ChainNodeAttribute(const ChainNodeAttributeArray& a) : value(a) {}
        ChainNodeAttribute(const ChainNodeAttribute& other) { value = other.value; }

        ChainNodeAttribute& operator=(const ChainNodeAttribute& other) {
            if (this != &other) {
                this->value = other.value;
            }
            return *this;
        }

        std::unique_ptr<ChainNodeAttribute> clone() const { 
            return std::make_unique<ChainNodeAttribute>(*this);
        }

        bool isNull() const { return std::holds_alternative<std::nullptr_t>(value); }
        bool isBool() const { return std::holds_alternative<bool>(value); }
        bool isDecimal() const { return std::holds_alternative<double>(value); }
        bool isInteger() const { return std::holds_alternative<int64_t>(value); }
        bool isString() const { return std::holds_alternative<std::string>(value); }
        bool isObject() const { return std::holds_alternative<ChainNodeAttributeObject>(value); }
        bool isArray() const { return std::holds_alternative<ChainNodeAttributeArray>(value); }

        ChainNodeAttributeVariant get() const { return value; }

        std::optional<std::nullptr_t> getNull() const {
            if (std::holds_alternative<std::nullptr_t>(value)) {
                return std::get<std::nullptr_t>(value);
            }
            return std::nullopt;
        }
        std::optional<bool> getBool() const {
            if (std::holds_alternative<bool>(value)) {
                return std::get<bool>(value);
            }
            return std::nullopt;
        }
        std::optional<double> getDecimal() const {
            if (std::holds_alternative<double>(value)) {
                return std::get<double>(value);
            }
            return std::nullopt;
        }
        std::optional<int64_t> getInteger() const {
            if (std::holds_alternative<int64_t>(value)) {
                return std::get<int64_t>(value);
            }
            return std::nullopt;
        }
        std::optional<std::string> getString() const {
            if (std::holds_alternative<std::string>(value)) {
                return std::get<std::string>(value);
            }
            return std::nullopt;
        }
        std::optional<ChainNodeAttributeObject> getObject() const {
            if (isObject()) return std::get<ChainNodeAttributeObject>(value);
            return std::nullopt;
        }
        std::optional<ChainNodeAttributeArray> getArray() const {
            if (isArray()) return std::get<ChainNodeAttributeArray>(value);
            return std::nullopt;
        }

        std::string to_string(size_t indent = 0) const {
            std::ostringstream oss;
            std::string indentStr(indent, ' '); 

            if (isNull()) {
                oss << "null";
            } else if (isBool()) {
                oss << (std::get<bool>(value) ? "true" : "false");
            } else if (isDecimal()) {
                oss << std::get<double>(value);
            } else if (isInteger()) {
                oss << std::get<int64_t>(value);
            } else if (isString()) {
                oss << "\"" << std::get<std::string>(value) << "\"";
            } else if (isObject()) {
                oss << "{";
                const auto& object = std::get<ChainNodeAttributeObject>(value);
                for (auto it = object.begin(); it != object.end(); ++it) {
                    if (it == object.begin()) {
                        oss << "\n";
                    }
                    auto elem = it->second;
                    oss << indentStr << "  \"" << it->first << "\": " << elem.to_string(indent + 2);
                    if (std::next(it) != object.end()) { 
                        oss << ",\n";
                    } else {
                        oss << "\n";
                        oss << indentStr;
                    }
                }
                oss << "}";
            } else if (isArray()) {
                oss << "[";
                const auto& array = std::get<ChainNodeAttributeArray>(value);
                for (auto it = array.begin(); it != array.end(); ++it) {
                    if (it == array.begin()) {
                        oss << "\n";
                    }
                    oss << indentStr << "  " << it->to_string(indent + 2);
                    if (std::next(it) != array.end()) { 
                        oss << ",\n";
                    } else {
                        oss << "\n";
                        oss << indentStr;
                    }
                }
                oss << "]";
            } else {
                oss << "-attribute type not supported-";
            }
            return oss.str();
        }

    }; 

}