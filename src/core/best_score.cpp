#include "core/best_score.h"

#include <stdexcept>

#include "nlohmann/json.hpp"

namespace kobo_2048::core {

using nlohmann::json;

std::string BestScore::toJson() const {
    return json{{"schemaVersion", 1}, {"value", value_}}.dump();
}

BestScore BestScore::fromJson(const std::string& text) {
    json j = json::parse(text);  // throws on malformed JSON
    if (!j.is_object() || j.value("schemaVersion", 0) != 1)
        throw std::runtime_error("invalid best score: schemaVersion");
    int64_t value = j.at("value").get<int64_t>();
    if (value < 0) throw std::runtime_error("invalid best score: negative value");

    BestScore b;
    b.value_ = static_cast<uint32_t>(value);
    return b;
}

}  // namespace kobo_2048::core
