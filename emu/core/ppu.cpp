#include <emu/core/ppu.hpp>

#include <cassert>
#include <cstring> // memset
#define DEBUG
#include <emu/utils/debug.hpp>

#define INSIDE_PPU_CPP

namespace Core {

void PPU::power(const ROM &chrrom, int mirroring)
{
    vram.power(chrrom, mirroring);
    bg.power();
    oam.power();
    nmi_enabled   = 0;
    ext_bus_dir   = 0;
    effects.grey  = 0;
    effects.red   = 0;
    effects.green = 0;
    effects.blue  = 0;
    // small note to myself:
    // most games check this in an infinite loop
    // to break out of the loop, set this to 1
    vblank  = 0;
    spr0hit = 0;
    sprov   = 1;
    latch.toggle = 0;
    odd_frame = 0;
    // randomize oam, palette, nt ram, chr ram
}

void PPU::reset()
{
    vram.reset();
    bg.reset();
    oam.reset();
    nmi_enabled   = 0;
    ext_bus_dir   = 0;
    effects.grey  = 0;
    effects.red   = 0;
    effects.green = 0;
    effects.blue  = 0;
    // spr0hit = random
    // sprov = random
    latch.toggle = 0;
    odd_frame = 0;
    // randomize oam
}

void PPU::begin_frame()
{
    if (odd) {
        cycles++;
        fetch_nt();
    }
}

/* where 0 <= Cycle <= 340 */
template <unsigned int Cycle>
void cycle()
{
    if constexpr((Cycle >= 1 && Cyle <= 255) || (Cycle >= 321 && Cycle <= 340)) {
        // fetches occur for 2 cycles. let's do them every odd cycle.
        if constexpr(Cycle % 8 == 1)
            cycle_fetchnt();
        if constexpr(Cycle % 8 == 3)
            cycle_fetchattr();
        if constexpr(Cycle % 8 == 5)
            cycle_fetchlowbg();
        if constexpr(Cycle % 8 == 7)
            cycle_fetchhighbg();
        if constexpr(Cycle % 8 == 0)
            cycle_incvhorz();
    }
    if constexpr(Cycle == 256)
        cycle_incvvert();
    if constexpr(Cycle == 257)
        cycle_copyhoriz();
}

static std::function<void(void)> cycletab[341] = {
    cycle<0>,   cycle<1>,   cycle<2>,   cycle<3>,   cycle<4>,   cycle<5>,   cycle<6>,   cycle<7>, 
    cycle<8>,   cycle<9>,   cycle<10>,  cycle<11>,  cycle<12>,  cycle<13>,  cycle<14>,  cycle<15>, 
    cycle<16>,  cycle<17>,  cycle<18>,  cycle<19>,  cycle<20>,  cycle<21>,  cycle<22>,  cycle<23>, 
    cycle<24>,  cycle<25>,  cycle<26>,  cycle<27>,  cycle<28>,  cycle<29>,  cycle<30>,  cycle<31>, 
    cycle<32>,  cycle<33>,  cycle<34>,  cycle<35>,  cycle<36>,  cycle<37>,  cycle<38>,  cycle<39>, 
    cycle<40>,  cycle<41>,  cycle<42>,  cycle<43>,  cycle<44>,  cycle<45>,  cycle<46>,  cycle<47>, 
    cycle<48>,  cycle<49>,  cycle<50>,  cycle<51>,  cycle<52>,  cycle<53>,  cycle<54>,  cycle<55>, 
    cycle<56>,  cycle<57>,  cycle<58>,  cycle<59>,  cycle<60>,  cycle<61>,  cycle<62>,  cycle<63>, 
    cycle<64>,  cycle<65>,  cycle<66>,  cycle<67>,  cycle<68>,  cycle<69>,  cycle<70>,  cycle<71>, 
    cycle<72>,  cycle<73>,  cycle<74>,  cycle<75>,  cycle<76>,  cycle<77>,  cycle<78>,  cycle<79>, 
    cycle<80>,  cycle<81>,  cycle<82>,  cycle<83>,  cycle<84>,  cycle<85>,  cycle<86>,  cycle<87>, 
    cycle<88>,  cycle<89>,  cycle<90>,  cycle<91>,  cycle<92>,  cycle<93>,  cycle<94>,  cycle<95>, 
    cycle<96>,  cycle<97>,  cycle<98>,  cycle<99>,  cycle<100>, cycle<101>, cycle<102>, cycle<103>, 
    cycle<104>, cycle<105>, cycle<106>, cycle<107>, cycle<108>, cycle<109>, cycle<110>, cycle<111>, 
    cycle<112>, cycle<113>, cycle<114>, cycle<115>, cycle<116>, cycle<117>, cycle<118>, cycle<119>, 
    cycle<120>, cycle<121>, cycle<122>, cycle<123>, cycle<124>, cycle<125>, cycle<126>, cycle<127>, 
    cycle<128>, cycle<129>, cycle<130>, cycle<131>, cycle<132>, cycle<133>, cycle<134>, cycle<135>, 
    cycle<136>, cycle<137>, cycle<138>, cycle<139>, cycle<140>, cycle<141>, cycle<142>, cycle<143>, 
    cycle<144>, cycle<145>, cycle<146>, cycle<147>, cycle<148>, cycle<149>, cycle<150>, cycle<151>, 
    cycle<152>, cycle<153>, cycle<154>, cycle<155>, cycle<156>, cycle<157>, cycle<158>, cycle<159>, 
    cycle<160>, cycle<161>, cycle<162>, cycle<163>, cycle<164>, cycle<165>, cycle<166>, cycle<167>, 
    cycle<168>, cycle<169>, cycle<170>, cycle<171>, cycle<172>, cycle<173>, cycle<174>, cycle<175>, 
    cycle<176>, cycle<177>, cycle<178>, cycle<179>, cycle<180>, cycle<181>, cycle<182>, cycle<183>, 
    cycle<184>, cycle<185>, cycle<186>, cycle<187>, cycle<188>, cycle<189>, cycle<190>, cycle<191>, 
    cycle<192>, cycle<193>, cycle<194>, cycle<195>, cycle<196>, cycle<197>, cycle<198>, cycle<199>, 
    cycle<200>, cycle<201>, cycle<202>, cycle<203>, cycle<204>, cycle<205>, cycle<206>, cycle<207>, 
    cycle<208>, cycle<209>, cycle<210>, cycle<211>, cycle<212>, cycle<213>, cycle<214>, cycle<215>, 
    cycle<216>, cycle<217>, cycle<218>, cycle<219>, cycle<220>, cycle<221>, cycle<222>, cycle<223>, 
    cycle<224>, cycle<225>, cycle<226>, cycle<227>, cycle<228>, cycle<229>, cycle<230>, cycle<231>, 
    cycle<232>, cycle<233>, cycle<234>, cycle<235>, cycle<236>, cycle<237>, cycle<238>, cycle<239>, 
    cycle<240>, cycle<241>, cycle<242>, cycle<243>, cycle<244>, cycle<245>, cycle<246>, cycle<247>, 
    cycle<248>, cycle<249>, cycle<250>, cycle<251>, cycle<252>, cycle<253>, cycle<254>, cycle<255>, 
    cycle<256>, cycle<257>, cycle<258>, cycle<259>, cycle<260>, cycle<261>, cycle<262>, cycle<263>, 
    cycle<264>, cycle<265>, cycle<266>, cycle<267>, cycle<268>, cycle<269>, cycle<270>, cycle<271>, 
    cycle<272>, cycle<273>, cycle<274>, cycle<275>, cycle<276>, cycle<277>, cycle<278>, cycle<279>, 
    cycle<280>, cycle<281>, cycle<282>, cycle<283>, cycle<284>, cycle<285>, cycle<286>, cycle<287>, 
    cycle<288>, cycle<289>, cycle<290>, cycle<291>, cycle<292>, cycle<293>, cycle<294>, cycle<295>, 
    cycle<296>, cycle<297>, cycle<298>, cycle<299>, cycle<300>, cycle<301>, cycle<302>, cycle<303>, 
    cycle<304>, cycle<305>, cycle<306>, cycle<307>, cycle<308>, cycle<309>, cycle<310>, cycle<311>, 
    cycle<312>, cycle<313>, cycle<314>, cycle<315>, cycle<316>, cycle<317>, cycle<318>, cycle<319>, 
    cycle<320>, cycle<321>, cycle<322>, cycle<323>, cycle<324>, cycle<325>, cycle<326>, cycle<327>, 
    cycle<328>, cycle<329>, cycle<330>, cycle<331>, cycle<332>, cycle<333>, cycle<334>, cycle<335>, 
    cycle<336>, cycle<337>, cycle<338>, cycle<339>, cycle<340>,
};

template <unsigned int Row>
void rowcycle(unsigned int col)
{
    if constexpr(Row < 240)
        cycletab[cycle]();
    if constexpr(Row >= 240 && Row != 261) {
        if (col == 241)
            vblank_begin();
    }
    if constexpr(Row == 261) {
        if (col == 1)
            vblank_end();
        if (col > 278 && col < 306)
            copy_vert();
        cycletab[col]();
    }
}

static std::function<void(unsigned int)> cyclerowtab[262] = {
    rowcycle<0>, rowcycle<1>, rowcycle<2>, rowcycle<3>, rowcycle<4>, 
    // TODO finish this dude
};

void PPU::main()
{
    cyclerowtab[cycles%340](cycles/340);
    cycles++;
}

uint8_t PPU::readreg(const uint16_t which)
{
    switch (which) {
    case 0x2000: case 0x2001: case 0x2003:
    case 0x2005: case 0x2006: case 0x4014:
        break;

    case 0x2002:
        io_latch |= (vblank << 7 | spr0hit << 6 | sprov << 5);
        vblank = 0;
        latch.toggle = 0;
        return io_latch;

    case 0x2004:
        io_latch = oam.data;
        return io_latch;

    case 0x2007:
        io_latch = vram.readdata();
        return io_latch;

#ifdef DEBUG
    default:
        assert(false);
        break;
#endif
    }
    // unreachable
    return io_latch;
}

void PPU::writereg(const uint16_t which, const uint8_t data)
{
    io_latch = data;
    switch (which) {
    case 0x2000:
        nmi_enabled         = data & 0x80;
        ext_bus_dir         = data & 0x40;
        vram.increment      = (data & 0x04) ? 32 : 1;
        oam.sprsize         = data & 0x20;
        bg.patterntab_addr  = data & 0x10;
        oam.patterntab_addr = data & 0x08;
        bg.nt_base_addr     = data & 0x03;
        vram.tmp.reg        = (data & 0x03) | (vram.tmp.reg & 0xF3FF);
        break;

    case 0x2001:
        effects.grey   = data & 0x01;
        bg.show_leftmost    = data & 0x02;
        oam.show_leftmost   = data & 0x04;
        bg.show             = data & 0x08;
        oam.show            = data & 0x10;
        effects.red         = data & 0x20;
        effects.green       = data & 0x40;
        effects.blue        = data & 0x80;
        break;

    case 0x2002:
        break;

    case 0x2003:
        oam.addr = data;
        break;

    case 0x2004:
        oam.data = data;
        oam.addr++;
        break;

    case 0x2005:
        if (latch.toggle == 0) {
            vram.tmp.reg = (data & 0xF8) | (vram.tmp.reg & 0xFFE0);
            vram.finex   = (data & 0x7);
        } else {
            vram.tmp.reg = (data & 0x07) | (vram.tmp.reg & 0x8FFF);
            vram.tmp.reg = (data & 0xF8) | (vram.tmp.reg & 0xFC1F);
        }
        latch.toggle ^= 1;
        break;

    case 0x2006:
        if (latch.toggle == 0) {
            vram.tmp.high = data & 0x3F;
        } else {
            vram.tmp.low = data;
            vram.vaddr = vram.tmp.reg;
        }
        latch.toggle ^= 1;
        break;

    case 0x2007:
        vram.writedata(data);
        break;

#ifdef DEBUG
    default:
        assert(false);
        break;
#endif
    }
}

#include <emu/core/vram.cpp>
#include <emu/core/background.cpp>
#include <emu/core/oam.cpp>

} // namespace Core

#undef INSIDE_PPU_CPP

