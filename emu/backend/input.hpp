#pragma once

#include <string_view>
#include <optional>
#include <bitset>
#include <emu/util/common.hpp>

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

class Keys {
    std::bitset<NUM_BUTTONS> pressed;
public:
    void clear()                         { pressed.reset(); }
    auto operator[](Button button)       { return pressed[static_cast<u32>(button)]; }
    auto operator[](Button button) const { return pressed[static_cast<u32>(button)]; }
};

} // namespace input
