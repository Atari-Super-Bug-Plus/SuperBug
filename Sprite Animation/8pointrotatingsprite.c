#include <atari.h>
#include <peekpoke.h>
#include <string.h>   /* memset / memcpy */

/* =====================================================================
   SuperBug 8‑Direction Rotation Demo (CC65 / Atari 8‑bit / Altirra)
   =====================================================================
   STATUS: 8 distinct orientation sprite slots are now defined (N, NE, E,
           SE, S, SW, W, NW).  Diagonal sprite *pixel data* currently uses
           provisional art (copied from nearest cardinal heading) so the
           program compiles and runs with the full 8‑step rotation system.
           You can edit the binary rows in-place for each diagonal to tune
           the look without touching code.

   CONTROLS:
     • Hold joystick RIGHT  → rotate clockwise through 8 headings.
     • Hold joystick LEFT   → rotate counter‑clockwise.
     • Both held            → no rotation (safety latch).

   BUILD:
     cl65 -t atari -o superbug8.xex superbug8.c

   NOTES:
     • Player 0 = left 8 pixels of car, Player 1 = right 8 pixels.
     • Sprite graphics are written starting at PMG + 50 scanlines (adjust
       ROW_OFFSET below if you need vertical repositioning).
     • Colors: COLOR0 & COLOR1 are currently both 254 (yellow/white-ish in
       NTSC palettes); tweak as desired.

   --------------------------------------------------------------------- */

/* ---------------- Configuration ------------------------------------- */
#define PMG_MEM     0x3000
#define PLAYER0_GFX (PMG_MEM + 0x200)
#define PLAYER1_GFX (PMG_MEM + 0x280)
#define ROW_OFFSET  50     /* # scanlines down from top of PM memory to draw */

/* ---------------- Hardware registers -------------------------------- */
#define HPOSP0  53248
#define HPOSP1  53249
#define SIZEP0  53256
#define SIZEP1  53257
#define GRACTL  53277
#define DMACTL  559
#define PMBASE  54279
#define COLOR0  704
#define COLOR1  705
#define STICK0  632

/* Joystick bit masks (active low) */
#define JS_UP    1
#define JS_DOWN  2
#define JS_LEFT  4
#define JS_RIGHT 8

/* Orientation indices */
#define ORIENT_N   0
#define ORIENT_NE  1
#define ORIENT_E   2
#define ORIENT_SE  3
#define ORIENT_S   4
#define ORIENT_SW  5
#define ORIENT_W   6
#define ORIENT_NW  7

/* frames between rotation steps while held */
#define ROT_DELAY 180

/* =====================================================================
   SPRITE DATA (16 rows each) -------------------------------------------------
   Each row shown in comments as: 8 bits Player0  SP  8 bits Player1.
   Edit the 0/1 patterns to redraw the car for that heading.  LSB is rightmost
   bit?  In cc65 the bit order is MSB=bit7 leftmost when poked to PMG memory,
   so write rows visually left->right as bits7..0.
   --------------------------------------------------------------------- */

/* --- N (Up) ---------------------------------------------------------- */
static const unsigned char car_N_p0[16] = {
    0b00000111,
    0b00001111,
    0b00011100,
    0b00011000,
    0b00011000,
    0b00011111,
    0b00001111,
    0b00001111,
    0b00001111,
    0b00011111,
    0b00011111,
    0b00011000,
    0b00011100,
    0b00001111,
    0b00000111,
    0b00000000,

};
static const unsigned char car_N_p1[16] = {
    0b11100000,
    0b11110000,
    0b00111000,
    0b00011000,
    0b00011000,
    0b11111000,
    0b11110000,
    0b11110000,
    0b11110000,
    0b11111000,
    0b11111000,
    0b00011000,
    0b00111000,
    0b11110000,
    0b11100000,
    0b00000000,

};

/* --- NE (provisional = N OR E mix; edit!) ---------------------------- */
static const unsigned char car_NE_p0[16] = {
    0b00000000,
    0b00000001,
    0b00000000,
    0b00000001,
    0b00000011,
    0b00000111,
    0b00001111,
    0b01011111,
    0b11110111,
    0b11100011,
    0b11110001,
    0b01111000,
    0b00111101,
    0b00011111,
    0b00001111,
    0b00000111,

};
static const unsigned char car_NE_p1[16] = {
    0b11100000,
    0b11111000,
    0b11111100,
    0b10011110,
    0b00001110,
    0b00000111,
    0b10000111,
    0b11001111,
    0b11111010,
    0b11110000,
    0b11100000,
    0b11000000,
    0b10000000,
    0b00000000,
    0b10000000,
    0b00000000,

};

/* --- E (Right) ------------------------------------------------------- */
static const unsigned char car_E_p0[16] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00011110,
    0b00111111,
    0b01110111,
    0b01100111,
    0b01100111,
    0b01100111,
    0b01100111,
    0b01110111,
    0b00111111,
    0b00011110,
    0b00000000,
    0b00000000,
    0b00000000,
};
static const unsigned char car_E_p1[16] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00111100,
    0b11111110,
    0b11100111,
    0b11100011,
    0b11100011,
    0b11100011,
    0b11100011,
    0b11100111,
    0b11111110,
    0b00111100,
    0b00000000,
    0b00000000,
    0b00000000,

};

/* --- SE (provisional = S OR E mix; edit!) ---------------------------- */
static const unsigned char car_SE_p0[16] = {
    0b00000111,
    0b00001111,
    0b00011111,
    0b00111101,
    0b01111000,
    0b11110001,
    0b11100011,
    0b11110111,
    0b01011111,
    0b00001111,
    0b00000111,
    0b00000011,
    0b00000001,
    0b00000000,
    0b00000001,
    0b00000000,

};
static const unsigned char car_SE_p1[16] = {
    0b00000000,
    0b10000000,
    0b00000000,
    0b10000000,
    0b11000000,
    0b11100000,
    0b11110000,
    0b11111010,
    0b11001111,
    0b10000111,
    0b00000111,
    0b00001110,
    0b10011110,
    0b11111100,
    0b11111000,
    0b11100000,

};

/* --- S (Down) -------------------------------------------------------- */
static const unsigned char car_S_p0[16] = {
    0b00000000,
    0b00000111,
    0b00001111,
    0b00011100,
    0b00011000,
    0b00011111,
    0b00011111,
    0b00001111,
    0b00001111,
    0b00001111,
    0b00011111,
    0b00011000,
    0b00011000,
    0b00011100,
    0b00001111,
    0b00000111,

};
static const unsigned char car_S_p1[16] = {
    0b00000000,
    0b11100000,
    0b11110000,
    0b00111000,
    0b00011000,
    0b11111000,
    0b11111000,
    0b11110000,
    0b11110000,
    0b11110000,
    0b11111000,
    0b00011000,
    0b00011000,
    0b00111000,
    0b11110000,
    0b11100000,

};

/* --- SW (provisional = S OR W mix; edit!) ---------------------------- */
static const unsigned char car_SW_p0[16] = {
    0b00000000,
    0b00000001,
    0b00000000,
    0b00000001,
    0b00000011,
    0b00000111,
    0b00001111,
    0b01011111,
    0b11110011,
    0b11100001,
    0b11100000,
    0b01110000,
    0b01111001,
    0b00111111,
    0b00011111,
    0b00000111,
};
static const unsigned char car_SW_p1[16] = {
    0b11100000,
    0b11110000,
    0b11111000,
    0b10111100,
    0b00011110,
    0b10001111,
    0b11000111,
    0b11101111,
    0b11111010,
    0b11110000,
    0b11100000,
    0b11000000,
    0b10000000,
    0b00000000,
    0b10000000,
    0b00000000,
};

/* --- W (Left) -------------------------------------------------------- */
static const unsigned char car_W_p0[16] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00111100,
    0b01111111,
    0b11100111,
    0b11000111,
    0b11000111,
    0b11000111,
    0b11000111,
    0b11100111,
    0b01111111,
    0b00111100,
    0b00000000,
    0b00000000,
    0b00000000,
};
static const unsigned char car_W_p1[16] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b01111000,
    0b11111100,
    0b11101110,
    0b11100110,
    0b11100110,
    0b11100110,
    0b11100110,
    0b11101110,
    0b11111100,
    0b01111000,
    0b00000000,
    0b00000000,
    0b00000000,

};

/* --- NW (provisional = N OR W mix; edit!) ---------------------------- */
static const unsigned char car_NW_p0[16] = {
    0b00000111,
    0b00011111,
    0b00111111,
    0b01111001,
    0b01110000,
    0b11100000,
    0b11100001,
    0b11110011,
    0b01011111,
    0b00001111,
    0b00000111,
    0b00000011,
    0b00000001,
    0b00000000,
    0b00000001,
    0b00000000,

};
static const unsigned char car_NW_p1[16] = {
    0b00000000,
    0b10000000,
    0b00000000,
    0b10000000,
    0b11000000,
    0b11100000,
    0b11110000,
    0b11111010,
    0b11101111,
    0b11000111,
    0b10001111,
    0b00011110,
    0b10111100,
    0b11111000,
    0b11110000,
    0b11100000,

};

/* =====================================================================
   GLOBAL STATE -------------------------------------------------------- */
static unsigned char xpos = 120;            /* horiz position */
static unsigned char orient = ORIENT_N;     /* current heading */

/* =====================================================================
   UTILS ---------------------------------------------------------------- */
static void clear_players(void)
{
    memset((void*)PLAYER0_GFX, 0, 256);
    memset((void*)PLAYER1_GFX, 0, 256);
}

/* loader helper */
static void load16(const unsigned char* src0, const unsigned char* src1)
{
    unsigned char i;
    for(i=0;i<16;++i){
        POKE(PLAYER0_GFX + ROW_OFFSET + i, src0[i]);
        POKE(PLAYER1_GFX + ROW_OFFSET + i, src1[i]);
    }
    POKE(HPOSP1, xpos + 8); /* keep right half aligned */
}

/* orientation dispatchers */
static void load_car_sprite(unsigned char o)
{
    switch(o & 7){
        case ORIENT_N:  load16(car_N_p0,  car_N_p1);  break;
        case ORIENT_NE: load16(car_NE_p0, car_NE_p1); break;
        case ORIENT_E:  load16(car_E_p0,  car_E_p1);  break;
        case ORIENT_SE: load16(car_SE_p0, car_SE_p1); break;
        case ORIENT_S:  load16(car_S_p0,  car_S_p1);  break;
        case ORIENT_SW: load16(car_SW_p0, car_SW_p1); break;
        case ORIENT_W:  load16(car_W_p0,  car_W_p1);  break;
        case ORIENT_NW: load16(car_NW_p0, car_NW_p1); break;
    }
}

/* =====================================================================
   MAIN ----------------------------------------------------------------- */
void main(void)
{
    unsigned char stick;
    unsigned char prev_orient = 0xFF; /* force first draw */
    unsigned char rot_cnt = 0;        /* counts frames between rotations */
    unsigned char rotate_cw;          /* joystick RIGHT */
    unsigned char rotate_ccw;         /* joystick LEFT  */

    /* --- PMG init (same as your original) --- */
    POKE(710, 0);
    POKE(DMACTL, 0x2E);
    POKE(GRACTL, 0x03);
    POKE(PMBASE, PMG_MEM >> 8);

    POKE(COLOR0, 254);
    POKE(COLOR1, 254);
    POKE(SIZEP0, 0);
    POKE(SIZEP1, 0);

    POKE(HPOSP0, xpos);

    clear_players();
    load_car_sprite(orient);
    prev_orient = orient;

    while(1){
        stick = PEEK(STICK0);

        /* active low */
        rotate_cw  = ((stick & JS_RIGHT) == 0);
        rotate_ccw = ((stick & JS_LEFT)  == 0);

        if(rotate_cw && !rotate_ccw){
            if(++rot_cnt >= ROT_DELAY){
                rot_cnt = 0;
                orient = (orient + 1) & 7; /* CW */
            }
        }else if(rotate_ccw && !rotate_cw){
            if(++rot_cnt >= ROT_DELAY){
                rot_cnt = 0;
                orient = (orient + 7) & 7; /* CCW */
            }
        }else{
            rot_cnt = 0; /* no rotation */
        }

        if(orient != prev_orient){
            load_car_sprite(orient);
            prev_orient = orient;
        }

        /* keep PMG halves aligned */
        POKE(HPOSP0, xpos);
        POKE(HPOSP1, xpos + 8);
    }
}

