#ifndef NESCPU_H_INCLUDED
#define NESCPU_H_INCLUDED

#define OPCODELEN 1

enum OPCODES : int {
    LDA = 0,
    STA = 1,
    LDX = 2,
    STX = 3,
};

namespace cpu {
    int decode(char opcode)
    {
        return opcode;
    }

    int fetchoperands1(char opcode)
    {
        return opcode;
    }
}

#endif

