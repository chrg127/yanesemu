void PPU::idlec()
{
    dbgputc('.');
}

// called at (340, 261)
void PPU::begin_frame()
{
    assert(lines%262 == 261 && cycles%341 == 340);
    if (odd_frame) {
        lines = 0;
        cycles = 0;
        dbgputc('\n');
    } else {
        dbgputc('0');
    }
    odd_frame^=1;
}

void PPU::cycle_fetchnt(bool cycle)
{
    bg.fetch_nt(cycle);
    dbgputc('n');
}

void PPU::cycle_fetchattr(bool cycle)
{
    bg.fetch_attr(cycle);
    dbgputc('a');
}

void PPU::cycle_fetchlowbg(bool cycle)
{
    bg.fetch_lowbg(cycle);
    dbgputc('l');
}

void PPU::cycle_fetchhighbg(bool cycle)
{
    bg.fetch_highbg(cycle);
    dbgputc('h');
}

void PPU::cycle_incvhorz()
{
    if (bg.show)
        vram.inc_horzpos();
    dbgputc('+');
}

void PPU::cycle_incvvert()
{
    if (bg.show)
        vram.inc_vertpos();
    dbgputc('^');
}

void PPU::cycle_copyhorz()
{
    if (bg.show)
        vram.copy_horzpos();
    dbgputc('c');
}

void PPU::cycle_copyvert()
{
    if (bg.show)
        vram.copy_vertpos();
    dbgputc('c');
}

void PPU::cycle_shift()
{
    bg.shift_run();
}

void PPU::cycle_fillshifts()
{
    bg.shift_fill();
}

void PPU::vblank_begin()
{
    vblank = 1;
    // call NMI interrupt here
    dbgputc('v');
}

void PPU::vblank_end()
{
    vblank  = 0;
    spr0hit = 0;
    spr_ov  = 0;
    dbgputc('e');
}

void PPU::cycle_outputpixel()
{
    output();
}

template <unsigned Cycle>
void PPU::background_cycle()
{
    if constexpr(Cycle == 1) cycle_fetchnt(0);
    if constexpr(Cycle == 2) cycle_fetchnt(1);
    if constexpr(Cycle == 3) cycle_fetchattr(0);
    if constexpr(Cycle == 4) cycle_fetchattr(1);
    if constexpr(Cycle == 5) cycle_fetchlowbg(0);
    if constexpr(Cycle == 6) cycle_fetchlowbg(1);
    if constexpr(Cycle == 7) cycle_fetchhighbg(0);
    if constexpr(Cycle == 0) cycle_fetchhighbg(1);
}

/* this only models lines from 1 to 239 */
template <unsigned int Cycle>
void PPU::ccycle()
{
    static_assert(Cycle <= 340);
    // NOTE: between cycle 257 - 320 there are garbage fetches
    if constexpr((Cycle >= 1 && Cycle <= 256) || (Cycle >= 321 && Cycle <= 340)) {
        cycle_outputpixel();
        background_cycle<Cycle % 8>();
        if constexpr(Cycle % 8 == 1 && Cycle != 1)   cycle_fillshifts();
        if constexpr(Cycle % 8 != 1)                 cycle_shift();
        if constexpr(Cycle % 8 == 0 && Cycle != 256) cycle_incvhorz();
        if constexpr(Cycle == 256)                   cycle_incvvert();
    }
    if constexpr(Cycle == 257) cycle_copyhorz();
}

using CycleFunc = void (PPU::*)();
using LineFunc  = void (PPU::*)(unsigned);

#define CCYCLE &PPU::ccycle
#define IDLE &PPU::idlec
static constexpr std::array<CycleFunc, 341> cycletab = {
    IDLE,        CCYCLE<1>,   CCYCLE<2>,   CCYCLE<3>,   CCYCLE<4>,   CCYCLE<5>,   CCYCLE<6>,   CCYCLE<7>,
    CCYCLE<8>,   CCYCLE<9>,   CCYCLE<10>,  CCYCLE<11>,  CCYCLE<12>,  CCYCLE<13>,  CCYCLE<14>,  CCYCLE<15>,
    CCYCLE<16>,  CCYCLE<17>,  CCYCLE<18>,  CCYCLE<19>,  CCYCLE<20>,  CCYCLE<21>,  CCYCLE<22>,  CCYCLE<23>,
    CCYCLE<24>,  CCYCLE<25>,  CCYCLE<26>,  CCYCLE<27>,  CCYCLE<28>,  CCYCLE<29>,  CCYCLE<30>,  CCYCLE<31>,
    CCYCLE<32>,  CCYCLE<33>,  CCYCLE<34>,  CCYCLE<35>,  CCYCLE<36>,  CCYCLE<37>,  CCYCLE<38>,  CCYCLE<39>,
    CCYCLE<40>,  CCYCLE<41>,  CCYCLE<42>,  CCYCLE<43>,  CCYCLE<44>,  CCYCLE<45>,  CCYCLE<46>,  CCYCLE<47>,
    CCYCLE<48>,  CCYCLE<49>,  CCYCLE<50>,  CCYCLE<51>,  CCYCLE<52>,  CCYCLE<53>,  CCYCLE<54>,  CCYCLE<55>,
    CCYCLE<56>,  CCYCLE<57>,  CCYCLE<58>,  CCYCLE<59>,  CCYCLE<60>,  CCYCLE<61>,  CCYCLE<62>,  CCYCLE<63>,
    CCYCLE<64>,  CCYCLE<65>,  CCYCLE<66>,  CCYCLE<67>,  CCYCLE<68>,  CCYCLE<69>,  CCYCLE<70>,  CCYCLE<71>,
    CCYCLE<72>,  CCYCLE<73>,  CCYCLE<74>,  CCYCLE<75>,  CCYCLE<76>,  CCYCLE<77>,  CCYCLE<78>,  CCYCLE<79>,
    CCYCLE<80>,  CCYCLE<81>,  CCYCLE<82>,  CCYCLE<83>,  CCYCLE<84>,  CCYCLE<85>,  CCYCLE<86>,  CCYCLE<87>,
    CCYCLE<88>,  CCYCLE<89>,  CCYCLE<90>,  CCYCLE<91>,  CCYCLE<92>,  CCYCLE<93>,  CCYCLE<94>,  CCYCLE<95>,
    CCYCLE<96>,  CCYCLE<97>,  CCYCLE<98>,  CCYCLE<99>,  CCYCLE<100>, CCYCLE<101>, CCYCLE<102>, CCYCLE<103>,
    CCYCLE<104>, CCYCLE<105>, CCYCLE<106>, CCYCLE<107>, CCYCLE<108>, CCYCLE<109>, CCYCLE<110>, CCYCLE<111>,
    CCYCLE<112>, CCYCLE<113>, CCYCLE<114>, CCYCLE<115>, CCYCLE<116>, CCYCLE<117>, CCYCLE<118>, CCYCLE<119>,
    CCYCLE<120>, CCYCLE<121>, CCYCLE<122>, CCYCLE<123>, CCYCLE<124>, CCYCLE<125>, CCYCLE<126>, CCYCLE<127>,
    CCYCLE<128>, CCYCLE<129>, CCYCLE<130>, CCYCLE<131>, CCYCLE<132>, CCYCLE<133>, CCYCLE<134>, CCYCLE<135>,
    CCYCLE<136>, CCYCLE<137>, CCYCLE<138>, CCYCLE<139>, CCYCLE<140>, CCYCLE<141>, CCYCLE<142>, CCYCLE<143>,
    CCYCLE<144>, CCYCLE<145>, CCYCLE<146>, CCYCLE<147>, CCYCLE<148>, CCYCLE<149>, CCYCLE<150>, CCYCLE<151>,
    CCYCLE<152>, CCYCLE<153>, CCYCLE<154>, CCYCLE<155>, CCYCLE<156>, CCYCLE<157>, CCYCLE<158>, CCYCLE<159>,
    CCYCLE<160>, CCYCLE<161>, CCYCLE<162>, CCYCLE<163>, CCYCLE<164>, CCYCLE<165>, CCYCLE<166>, CCYCLE<167>,
    CCYCLE<168>, CCYCLE<169>, CCYCLE<170>, CCYCLE<171>, CCYCLE<172>, CCYCLE<173>, CCYCLE<174>, CCYCLE<175>,
    CCYCLE<176>, CCYCLE<177>, CCYCLE<178>, CCYCLE<179>, CCYCLE<180>, CCYCLE<181>, CCYCLE<182>, CCYCLE<183>,
    CCYCLE<184>, CCYCLE<185>, CCYCLE<186>, CCYCLE<187>, CCYCLE<188>, CCYCLE<189>, CCYCLE<190>, CCYCLE<191>,
    CCYCLE<192>, CCYCLE<193>, CCYCLE<194>, CCYCLE<195>, CCYCLE<196>, CCYCLE<197>, CCYCLE<198>, CCYCLE<199>,
    CCYCLE<200>, CCYCLE<201>, CCYCLE<202>, CCYCLE<203>, CCYCLE<204>, CCYCLE<205>, CCYCLE<206>, CCYCLE<207>,
    CCYCLE<208>, CCYCLE<209>, CCYCLE<210>, CCYCLE<211>, CCYCLE<212>, CCYCLE<213>, CCYCLE<214>, CCYCLE<215>,
    CCYCLE<216>, CCYCLE<217>, CCYCLE<218>, CCYCLE<219>, CCYCLE<220>, CCYCLE<221>, CCYCLE<222>, CCYCLE<223>,
    CCYCLE<224>, CCYCLE<225>, CCYCLE<226>, CCYCLE<227>, CCYCLE<228>, CCYCLE<229>, CCYCLE<230>, CCYCLE<231>,
    CCYCLE<232>, CCYCLE<233>, CCYCLE<234>, CCYCLE<235>, CCYCLE<236>, CCYCLE<237>, CCYCLE<238>, CCYCLE<239>,
    CCYCLE<240>, CCYCLE<241>, CCYCLE<242>, CCYCLE<243>, CCYCLE<244>, CCYCLE<245>, CCYCLE<246>, CCYCLE<247>,
    CCYCLE<248>, CCYCLE<249>, CCYCLE<250>, CCYCLE<251>, CCYCLE<252>, CCYCLE<253>, CCYCLE<254>, CCYCLE<255>,
    CCYCLE<256>, CCYCLE<257>, IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,
    IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,
    IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,
    IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,
    IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,
    IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,
    IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,
    IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,        IDLE,
    IDLE,        CCYCLE<321>, CCYCLE<322>, CCYCLE<323>, CCYCLE<324>, CCYCLE<325>, CCYCLE<326>, CCYCLE<327>,
    CCYCLE<328>, CCYCLE<329>, CCYCLE<330>, CCYCLE<331>, CCYCLE<332>, CCYCLE<333>, CCYCLE<334>, CCYCLE<335>,
    CCYCLE<336>, CCYCLE<337>, CCYCLE<338>, CCYCLE<339>, CCYCLE<340>,
};
#undef CCYCLE
#undef IDLE

template <unsigned int Line>
void PPU::lcycle(unsigned int cycle)
{
    static_assert(Line <= 261);
    assert(cycle <= 340);
    if (cycle == 0) {
        dbgputc('\n');
    }
    if constexpr(Line < 240) {
        const auto f = cycletab[cycle];
        (this->*f)();
    }
    if constexpr(Line == 241)
        cycle == 1 ? vblank_begin() : idlec();
    if constexpr(Line == 261) {
        const auto f = cycletab[cycle];
        (this->*f)();
        if (cycle == 1)                   vblank_end();
        if (cycle >= 280 && cycle <= 304) cycle_copyvert();
        if (cycle == 340)                 begin_frame();
    }
    if constexpr(Line == 240 || (Line >= 242 && Line != 261))
        idlec();
}

#define LCYCLE &PPU::lcycle
static constexpr std::array<LineFunc, 262> linetab = {
    LCYCLE<0>,   LCYCLE<1>,   LCYCLE<2>,   LCYCLE<3>,   LCYCLE<4>,   LCYCLE<5>,   LCYCLE<6>,   LCYCLE<7>,
    LCYCLE<8>,   LCYCLE<9>,   LCYCLE<10>,  LCYCLE<11>,  LCYCLE<12>,  LCYCLE<13>,  LCYCLE<14>,  LCYCLE<15>,
    LCYCLE<16>,  LCYCLE<17>,  LCYCLE<18>,  LCYCLE<19>,  LCYCLE<20>,  LCYCLE<21>,  LCYCLE<22>,  LCYCLE<23>,
    LCYCLE<24>,  LCYCLE<25>,  LCYCLE<26>,  LCYCLE<27>,  LCYCLE<28>,  LCYCLE<29>,  LCYCLE<30>,  LCYCLE<31>,
    LCYCLE<32>,  LCYCLE<33>,  LCYCLE<34>,  LCYCLE<35>,  LCYCLE<36>,  LCYCLE<37>,  LCYCLE<38>,  LCYCLE<39>,
    LCYCLE<40>,  LCYCLE<41>,  LCYCLE<42>,  LCYCLE<43>,  LCYCLE<44>,  LCYCLE<45>,  LCYCLE<46>,  LCYCLE<47>,
    LCYCLE<48>,  LCYCLE<49>,  LCYCLE<50>,  LCYCLE<51>,  LCYCLE<52>,  LCYCLE<53>,  LCYCLE<54>,  LCYCLE<55>,
    LCYCLE<56>,  LCYCLE<57>,  LCYCLE<58>,  LCYCLE<59>,  LCYCLE<60>,  LCYCLE<61>,  LCYCLE<62>,  LCYCLE<63>,
    LCYCLE<64>,  LCYCLE<65>,  LCYCLE<66>,  LCYCLE<67>,  LCYCLE<68>,  LCYCLE<69>,  LCYCLE<70>,  LCYCLE<71>,
    LCYCLE<72>,  LCYCLE<73>,  LCYCLE<74>,  LCYCLE<75>,  LCYCLE<76>,  LCYCLE<77>,  LCYCLE<78>,  LCYCLE<79>,
    LCYCLE<80>,  LCYCLE<81>,  LCYCLE<82>,  LCYCLE<83>,  LCYCLE<84>,  LCYCLE<85>,  LCYCLE<86>,  LCYCLE<87>,
    LCYCLE<88>,  LCYCLE<89>,  LCYCLE<90>,  LCYCLE<91>,  LCYCLE<92>,  LCYCLE<93>,  LCYCLE<94>,  LCYCLE<95>,
    LCYCLE<96>,  LCYCLE<97>,  LCYCLE<98>,  LCYCLE<99>,  LCYCLE<100>, LCYCLE<101>, LCYCLE<102>, LCYCLE<103>,
    LCYCLE<104>, LCYCLE<105>, LCYCLE<106>, LCYCLE<107>, LCYCLE<108>, LCYCLE<109>, LCYCLE<110>, LCYCLE<111>,
    LCYCLE<112>, LCYCLE<113>, LCYCLE<114>, LCYCLE<115>, LCYCLE<116>, LCYCLE<117>, LCYCLE<118>, LCYCLE<119>,
    LCYCLE<120>, LCYCLE<121>, LCYCLE<122>, LCYCLE<123>, LCYCLE<124>, LCYCLE<125>, LCYCLE<126>, LCYCLE<127>,
    LCYCLE<128>, LCYCLE<129>, LCYCLE<130>, LCYCLE<131>, LCYCLE<132>, LCYCLE<133>, LCYCLE<134>, LCYCLE<135>,
    LCYCLE<136>, LCYCLE<137>, LCYCLE<138>, LCYCLE<139>, LCYCLE<140>, LCYCLE<141>, LCYCLE<142>, LCYCLE<143>,
    LCYCLE<144>, LCYCLE<145>, LCYCLE<146>, LCYCLE<147>, LCYCLE<148>, LCYCLE<149>, LCYCLE<150>, LCYCLE<151>,
    LCYCLE<152>, LCYCLE<153>, LCYCLE<154>, LCYCLE<155>, LCYCLE<156>, LCYCLE<157>, LCYCLE<158>, LCYCLE<159>,
    LCYCLE<160>, LCYCLE<161>, LCYCLE<162>, LCYCLE<163>, LCYCLE<164>, LCYCLE<165>, LCYCLE<166>, LCYCLE<167>,
    LCYCLE<168>, LCYCLE<169>, LCYCLE<170>, LCYCLE<171>, LCYCLE<172>, LCYCLE<173>, LCYCLE<174>, LCYCLE<175>,
    LCYCLE<176>, LCYCLE<177>, LCYCLE<178>, LCYCLE<179>, LCYCLE<180>, LCYCLE<181>, LCYCLE<182>, LCYCLE<183>,
    LCYCLE<184>, LCYCLE<185>, LCYCLE<186>, LCYCLE<187>, LCYCLE<188>, LCYCLE<189>, LCYCLE<190>, LCYCLE<191>,
    LCYCLE<192>, LCYCLE<193>, LCYCLE<194>, LCYCLE<195>, LCYCLE<196>, LCYCLE<197>, LCYCLE<198>, LCYCLE<199>,
    LCYCLE<200>, LCYCLE<201>, LCYCLE<202>, LCYCLE<203>, LCYCLE<204>, LCYCLE<205>, LCYCLE<206>, LCYCLE<207>,
    LCYCLE<208>, LCYCLE<209>, LCYCLE<210>, LCYCLE<211>, LCYCLE<212>, LCYCLE<213>, LCYCLE<214>, LCYCLE<215>,
    LCYCLE<216>, LCYCLE<217>, LCYCLE<218>, LCYCLE<219>, LCYCLE<220>, LCYCLE<221>, LCYCLE<222>, LCYCLE<223>,
    LCYCLE<224>, LCYCLE<225>, LCYCLE<226>, LCYCLE<227>, LCYCLE<228>, LCYCLE<229>, LCYCLE<230>, LCYCLE<231>,
    LCYCLE<232>, LCYCLE<233>, LCYCLE<234>, LCYCLE<235>, LCYCLE<236>, LCYCLE<237>, LCYCLE<238>, LCYCLE<239>,
    LCYCLE<240>, LCYCLE<241>, LCYCLE<242>, LCYCLE<243>, LCYCLE<244>, LCYCLE<245>, LCYCLE<246>, LCYCLE<247>,
    LCYCLE<248>, LCYCLE<249>, LCYCLE<250>, LCYCLE<251>, LCYCLE<252>, LCYCLE<253>, LCYCLE<254>, LCYCLE<255>,
    LCYCLE<256>, LCYCLE<257>, LCYCLE<258>, LCYCLE<259>, LCYCLE<260>, LCYCLE<261>
};
#undef LCYCLE

void PPU::run()
{
    const auto linefunc = linetab[lines % 262];
    (this->*linefunc)(cycles % 341);
    cycles++;
    lines += (cycles % 341 == 0);
}

