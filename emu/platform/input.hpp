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

inline std::optional<input::Button> string_to_button(std::string_view str)
{
    if (str == "a" || str == "A") return input::Button::A;
    if (str == "b" || str == "B") return input::Button::B;
    if (str == "select")          return input::Button::Select;
    if (str == "start")           return input::Button::Start;
    if (str == "up")              return input::Button::Up;
    if (str == "down")            return input::Button::Down;
    if (str == "left")            return input::Button::Left;
    if (str == "right")           return input::Button::Right;
    return std::nullopt;
}


class ButtonArray {
    std::array<bool, NUM_BUTTONS> buttons_pressed;
public:
    ButtonArray()                    { clear(); }
    void clear()                     { std::fill(buttons_pressed.begin(), buttons_pressed.end(), false); }
    bool & operator[](Button button) { return buttons_pressed[static_cast<int>(button)]; }
    void dump()
    {
        for (std::size_t i = 0; i < buttons_pressed.size(); i++) {
            auto button = static_cast<Button>(i);
            auto str = button_to_string(button);
            fmt::print("key: {}, pressed: {}\n", str, buttons_pressed[i]);
        }
    }
};

} // namespace input
