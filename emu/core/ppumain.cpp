template <unsigned Cycle>
void PPU::background_fetch_cycle()
{
    if constexpr(Cycle == 1) vram.buf     = fetch_nt(vram.addr.v);
    if constexpr(Cycle == 2) tile.nt      = vram.buf;
    if constexpr(Cycle == 3) vram.buf     = fetch_attr(vram.addr.nt, vram.addr.coarse_y, vram.addr.coarse_x);
    if constexpr(Cycle == 4) tile.attr    = vram.buf;
    if constexpr(Cycle == 5) vram.buf     = fetch_pt(io.bg_pt_addr, tile.nt, 0, u8(vram.addr.fine_y));
    if constexpr(Cycle == 6) tile.pt_low  = vram.buf;
    if constexpr(Cycle == 7) vram.buf     = fetch_pt(io.bg_pt_addr, tile.nt, 1, u8(vram.addr.fine_y));
    if constexpr(Cycle == 0) tile.pt_high = vram.buf;
}

template <unsigned Cycle>
void PPU::sprite_fetch_cycle(u3 n, unsigned line)
{
    if constexpr(Cycle == 1) vram.buf       = fetch_nt(0);
    // nothing happens on cycle 2
    if constexpr(Cycle == 3) { vram.buf     = fetch_attr(0, 0, 0); oam.attrs[n] = sprite.attr; }
    if constexpr(Cycle == 4) oam.xpos[n]    = sprite.x;
    if constexpr(Cycle == 5) vram.buf       = fetch_pt_sprite(io.sp_size, sprite.nt, 0, line - sprite.y);
    if constexpr(Cycle == 6) oam.pt_low[n]  = vram.buf;
    if constexpr(Cycle == 7) vram.buf       = fetch_pt_sprite(io.sp_size, sprite.nt, 1, line - sprite.y);
    if constexpr(Cycle == 0) oam.pt_high[n] = vram.buf;
}

template <unsigned Cycle>
void PPU::sprite_read_secondary()
{
    if constexpr(Cycle == 1) sprite.y    = secondary_oam.mem[secondary_oam.index++];
    if constexpr(Cycle == 2) sprite.nt   = secondary_oam.mem[secondary_oam.index++];
    if constexpr(Cycle == 3) sprite.attr = secondary_oam.mem[secondary_oam.index++];
    if constexpr(Cycle == 4) sprite.x    = secondary_oam.mem[secondary_oam.index  ];
    if constexpr(Cycle == 5) oam.data    = secondary_oam.mem[secondary_oam.index  ];
    if constexpr(Cycle == 6) oam.data    = secondary_oam.mem[secondary_oam.index  ];
    if constexpr(Cycle == 7) oam.data    = secondary_oam.mem[secondary_oam.index  ];
    if constexpr(Cycle == 0) oam.data    = secondary_oam.mem[secondary_oam.index++];
}

template <unsigned Cycle>
void PPU::cycle(unsigned line)
{
    if constexpr(Cycle == 0) {
        if (oam.sp0_next) {
            oam.sp0_curr = 1;
            oam.sp0_next = 0;
        }
    }
    if constexpr(Cycle >= 1 && Cycle <= 256) {
        if (line != 261)
            render();
    }

    if (io.bg_show) {
        if constexpr((Cycle >= 1 && Cycle <= 256) || (Cycle >= 321 && Cycle <= 336)) {
            if constexpr(Cycle % 8 == 1) background_shift_fill();
            background_shift_run();
            background_fetch_cycle<Cycle % 8>();
            if constexpr(Cycle % 8 == 0) vram.addr = inc_v_horzpos(vram.addr);
            if constexpr(Cycle == 256)   vram.addr = inc_v_vertpos(vram.addr);
        }
        if constexpr(Cycle == 257) copy_v_horzpos();
        if constexpr(Cycle >= 337 && Cycle <= 340)
            background_fetch_cycle<Cycle % 8>();
    }

    if (io.sp_show) {
        if constexpr(Cycle >= 1 && Cycle <= 256)
            sprite_shift_run();
        if constexpr(Cycle >= 257 && Cycle <= 320) {
            if constexpr(Cycle == 257) secondary_oam.index = 0;
            u3 sprite_num = util::getbits(secondary_oam.index, 2, 3);
            sprite_read_secondary<Cycle % 8>();
            sprite_fetch_cycle<Cycle % 8>(sprite_num, line);
            oam.addr = 0;
        }
    }

    if (io.sp_show && line != 261) {
        if constexpr(Cycle >= 1 && Cycle <= 64) {
            if constexpr(Cycle == 1) { oam.read_ff = 1; secondary_oam.index = 0; }
            if constexpr(Cycle % 2 == 1) { oam.data = oam.read(); }
            if constexpr(Cycle % 2 == 0) { secondary_oam.write(oam.data); secondary_oam.inc(); }
            if constexpr(Cycle == 64) {
                oam.read_ff = 0;
                oam.sp_counter = 0;
                secondary_oam.index = 0;
            }
        }
        if constexpr(Cycle >= 65 && Cycle <= 256) {
            if constexpr(Cycle % 2 == 1) { oam.data = oam.read(); }
            if constexpr(Cycle % 2 == 0) {
                secondary_oam.write(oam.data);
                sprite_update_flags(line);
            }
        }
        if constexpr((Cycle >= 321 && Cycle <= 340) || Cycle == 0) {
            oam.data = secondary_oam.mem[secondary_oam.index];
        }
    }
}

using CycleFunc = void (PPU::*)(unsigned);
using LineFunc  = void (PPU::*)(unsigned, CycleFunc);

template <unsigned Line>
void PPU::line(unsigned cycle, CycleFunc cycle_fn)
{
    if constexpr(Line < 240)
        (this->*cycle_fn)(Line);
    if constexpr(Line == 241)
        if (cycle == 1)
            vblank_begin();
    if constexpr(Line == 261) {
        (this->*cycle_fn)(Line);
        if (cycle == 1)
            vblank_end();
        if (io.bg_show && cycle >= 280 && cycle <= 304)
            copy_v_vertpos();
        // check for last cycle.
        // last cycle of this scanline is 339 on an odd_frame, 340 on an even frame
        if (cycle == 340u - odd_frame) {
            if (odd_frame)
                cycle_inc();
            odd_frame ^= 1;
        }
    }
}

void PPU::run()
{
#define LCYCLE &PPU::line
    static constexpr std::array<LineFunc, PPU_MAX_LINES> linetab = {
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

#define CCYCLE &PPU::cycle
static constexpr std::array<CycleFunc, PPU_MAX_LCYCLE> cycletab = {
    CCYCLE<0>,   CCYCLE<1>,   CCYCLE<2>,   CCYCLE<3>,   CCYCLE<4>,   CCYCLE<5>,   CCYCLE<6>,   CCYCLE<7>,
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
    CCYCLE<256>, CCYCLE<257>, CCYCLE<258>, CCYCLE<259>, CCYCLE<260>, CCYCLE<261>, CCYCLE<262>, CCYCLE<263>,
    CCYCLE<264>, CCYCLE<265>, CCYCLE<266>, CCYCLE<267>, CCYCLE<268>, CCYCLE<269>, CCYCLE<270>, CCYCLE<271>,
    CCYCLE<272>, CCYCLE<273>, CCYCLE<274>, CCYCLE<275>, CCYCLE<276>, CCYCLE<277>, CCYCLE<278>, CCYCLE<279>,
    CCYCLE<280>, CCYCLE<281>, CCYCLE<282>, CCYCLE<283>, CCYCLE<284>, CCYCLE<285>, CCYCLE<286>, CCYCLE<287>,
    CCYCLE<288>, CCYCLE<289>, CCYCLE<290>, CCYCLE<291>, CCYCLE<292>, CCYCLE<293>, CCYCLE<294>, CCYCLE<295>,
    CCYCLE<296>, CCYCLE<297>, CCYCLE<298>, CCYCLE<299>, CCYCLE<300>, CCYCLE<301>, CCYCLE<302>, CCYCLE<303>,
    CCYCLE<304>, CCYCLE<305>, CCYCLE<306>, CCYCLE<307>, CCYCLE<308>, CCYCLE<309>, CCYCLE<310>, CCYCLE<311>,
    CCYCLE<312>, CCYCLE<313>, CCYCLE<314>, CCYCLE<315>, CCYCLE<316>, CCYCLE<317>, CCYCLE<318>, CCYCLE<319>,
    CCYCLE<320>, CCYCLE<321>, CCYCLE<322>, CCYCLE<323>, CCYCLE<324>, CCYCLE<325>, CCYCLE<326>, CCYCLE<327>,
    CCYCLE<328>, CCYCLE<329>, CCYCLE<330>, CCYCLE<331>, CCYCLE<332>, CCYCLE<333>, CCYCLE<334>, CCYCLE<335>,
    CCYCLE<336>, CCYCLE<337>, CCYCLE<338>, CCYCLE<339>, CCYCLE<340>,
};
#undef CCYCLE

    const auto linefn  = linetab[lines];
    const auto cyclefn = cycletab[cycles];
    (this->*linefn)(cycles, cyclefn);
    cycle_inc();
}
