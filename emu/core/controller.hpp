#pragma once

#include <memory>
#include <emu/util/debug.hpp>
#include <emu/util/uint.hpp>

/*
 * A NES has 2 controller ports (for player 1 and player 2). The CPU is
 * connected to this port through the OUT pins, and the port is turn connected
 * to the controller device (of course).
 * Controller devices have two states: serial and parallel. On serial mode, we
 * can read the controller (by reading $4016/$4017); parallel mode polls for
 * input. Writing to $4016 is what controls these states (and latch() is the
 * corresponding function).
 */

namespace core {

struct Controller {
    enum class Type {
        Gamepad,
    };
    virtual u8 read() = 0;
    virtual void latch(bool state) = 0;
};

class Gamepad : public Controller {
    bool latched = 0;
    u8 buttons;
public:
    u8 read();
    void latch(bool state);
};

struct ControllerPort {
    std::unique_ptr<Controller> device;

    void load(Controller::Type type)
    {
        switch (type) {
        case Controller::Type::Gamepad: device = std::make_unique<Gamepad>(); break;
        default: panic("at {}\n", __func__);
        }
    }
};

} // namespace core
