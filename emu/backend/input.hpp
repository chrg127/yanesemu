#pragma once

#include <string_view>
#include <optional>
#include <array>
#include <fmt/core.h>

namespace input {

enum class Button {
    A,
    B,
    Start,
    Select,
    Up,
    Down,
    Left,
    Right,
};

const int NUM_BUTTONS = 8;

constexpr inline std::string_view button_to_string(Button button)
{
    switch (button) {
    case Button::A:      return "A";
    case Button::B:      return "B";
    case Button::Start:  return "Start";
    case Button::Select: return "Select";
    case Button::Up:     return "Up";
    case Button::Down:   return "Down";
    case Button::Left:   return "Left";
    case Button::Right:  return "Right";
    default:             return "";
    }
}

inline std::optional<Button> string_to_button(std::string_view str)
{
    if (str == "a" || str == "A") return Button::A;
    if (str == "b" || str == "B") return Button::B;
    if (str == "select")          return Button::Select;
    if (str == "start")           return Button::Start;
    if (str == "up")              return Button::Up;
    if (str == "down")            return Button::Down;
    if (str == "left")            return Button::Left;
    if (str == "right")           return Button::Right;
    return std::nullopt;
}

class ButtonArray {
    std::array<bool, NUM_BUTTONS> pressed = {};
public:
    void clear()                     { std::fill(pressed.begin(), pressed.end(), false); }
    bool & operator[](Button button) { return pressed[static_cast<int>(button)]; }
    void dump()
    {
        for (auto i = 0u; i < pressed.size(); i++) {
            auto button = static_cast<Button>(i);
            auto str = button_to_string(button);
            fmt::print("key: {}, pressed: {}\n", str, pressed[i]);
        }
    }
};

} // namespace input
