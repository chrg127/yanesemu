#ifndef CORE_DEBUGGER_HPP_INCLUDED
#define CORE_DEBUGGER_HPP_INCLUDED

#include <vector>
#include <emu/util/unsigned.hpp>
#include <emu/util/file.hpp>
#include <emu/core/emulator.hpp>

namespace Core {

/* This debugger is an owning class. A new main loop should be made for it to be
 * used. */
class Debugger {
    struct Breakpoint {
        uint16 start;
        uint16 end;
    };

    Util::File input {stdin};
    Emulator emu;
    std::vector<Breakpoint> breakpoints;
    bool parsing = true;
    bool quit = false;

public:
    Debugger(Emulator &&e)
        : emu(std::move(e))
    { }

    void run();
    void repl();
    bool got_quit() const { return quit; }

private:
    void print_help();
};

} // namespace Core

#endif
