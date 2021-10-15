#pragma once

#include <string_view>
#include <fmt/core.h>

namespace input {

enum class Button {
    JUMP,
    RUN,
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

const int NUM_BUTTONS = 6;

constexpr inline std::string_view button_to_string(Button button)
{
    switch (button) {
    case Button::JUMP:  return "jump";
    case Button::RUN:   return "run";
    case Button::UP:    return "up";
    case Button::DOWN:  return "down";
    case Button::LEFT:  return "left";
    case Button::RIGHT: return "right";
    default: return "";
    }
}

class Keys {
    std::array<bool, NUM_BUTTONS> buttons_pressed;
public:
    void clear()                            { std::fill(buttons_pressed.begin(), buttons_pressed.end(), false); }
    // void set(Button button, bool value)     { buttons_pressed[static_cast<int>(button)] = value; }
    // bool is_pressed(Button button) const    { return buttons_pressed[static_cast<int>(button)]; }
    bool & operator[](Button button)        { return buttons_pressed[static_cast<int>(button)]; }
    void dump()
    {
        for (std::size_t i = 0; i < buttons_pressed.size(); i++) {
            Button button = static_cast<Button>(i);
            auto str = button_to_string(button);
            fmt::print("key: {}, pressed: {}\n", str, buttons_pressed[i]);
        }
    }
};

} // namespace input
