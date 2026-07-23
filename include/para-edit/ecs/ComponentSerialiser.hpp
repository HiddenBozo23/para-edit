#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace para {

class ComponentSerialiser {
   public:
    template <typename T>
    static std::string SerialiseComponent(const T& component) {
        nlohmann::json j = component;  // calls to_json(json&, const T&) via ADL
        return j.dump();
    }

    template <typename T>
    static T DeserialiseComponent(const std::string& string) {
        nlohmann::json j = nlohmann::json::parse(string);
        return j.get<T>();  // calls from_json(const json&, T&) via ADL
    }
};

}  // namespace para