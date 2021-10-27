#include "controller.hpp"

#include <emu/program.hpp>
#include <emu/platform/input.hpp> // Button

using input::Button;

namespace core {

u8 Gamepad::read()
{
    if (latched == 1)
        return program.poll_input(Button::A);
    auto bit = buttons & 1;
    buttons >>= 1;
    return bit;
}

void Gamepad::latch(bool state)
{
    if (latched == state)
        return;
    latched = state;
    if (latched == 0) {
        for (auto &button : { Button::Right,  Button::Left,  Button::Down, Button::Up,
                              Button::Select, Button::Start, Button::B,    Button::A }) {
            buttons <<= 1;
            buttons |= program.poll_input(button);
        }
    }
}

} // namespace core
