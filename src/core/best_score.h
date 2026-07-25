#pragma once
#include <cstdint>
#include <string>

namespace kobo_2048::core {

// Highest current score ever reached, independent of any single game
// session (FR-008). update() enforces the monotonic-increase rule so
// callers never need to compare before calling it.
class BestScore {
public:
    uint32_t value() const { return value_; }
    void update(uint32_t score) {
        if (score > value_) value_ = score;
    }

    std::string toJson() const;
    static BestScore fromJson(const std::string& text);  // throws on invalid input

private:
    uint32_t value_ = 0;
};

}  // namespace kobo_2048::core
