#include "controller.hpp"

#include <emu/program.hpp>
#include <emu/platform/input.hpp> // Button

using input::Button;

namespace core {

// enum class Button {
//     A, B, Start, Select, Right, Left, Down, Up,
// };

// bool poll_input(Button button);

u8 Gamepad::read()
{
    if (latched == 0)
        return program.poll_input(Button::A);
    buttons >>= 1;
    return buttons & 1;
}

void Gamepad::latch(bool state)
{
    if (latched == state)
        return;
    latched = state;
    if (latched == 0) {
        static const Button lookup[] = { Button::Right,  Button::Left,  Button::Down, Button::Up,
                                         Button::Select, Button::Start, Button::A,    Button::B };
        for (std::size_t i = 0; i < std::size(lookup); i++) {
            buttons |= program.poll_input(lookup[i]);
            buttons <<= 1;
        }
    }
}

} // namespace core
