#include "controller.hpp"

#include <emu/program.hpp>

using input::Button;

namespace core {

u8 Gamepad::read()
{
    if (latched == 1)
        return program.poll_input()[Button::A];
    auto bit = buttons & 1;
    buttons >>= 1;
    return bit;
}

void Gamepad::latch(bool state)
{
    if (latched == state)
        return;
    latched = state;
    // poll input only when we switch to serial mode
    if (latched == 0) {
        auto curr_keys = program.poll_input();
        for (auto button : { Button::Right, Button::Left,   Button::Down, Button::Up,
                             Button::Start, Button::Select, Button::B,    Button::A }) {
            buttons <<= 1;
            buttons |= bool(curr_keys[button]) | bool(hold_buttons[button]);
        }
    }
}

} // namespace core
