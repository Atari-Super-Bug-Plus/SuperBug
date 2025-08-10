#include <atari.h>
#include <_antic.h>
#include <_atarios.h>
#include <peekpoke.h>
#include <conio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <joystick.h>
#include <string.h>

#define CUSTOM_CHARSET ((unsigned char*)0xB800)         // Must be 1K aligned and not overlap with CHARMAP
#define CHARMAP_ADDR  0x6400
#define SCREEN_ADDR   ((unsigned char*)0xB400)
#define DISPLAY_LIST  0x6000
#define SPRITE_TABLE_ADDR ((unsigned char*)0xBF40)
#define HUD_BUFFER ((unsigned char*)0xBE00)
#define SPRITE_BUFFER ((unsigned char*)0x0500)  // Pick a safe area not used by anything else


#define SPRITE_N_P0     (SPRITE_BUFFER + 0x00)
#define SPRITE_N_P1     (SPRITE_BUFFER + 0x10)
#define SPRITE_NE_P0    (SPRITE_BUFFER + 0x20)
#define SPRITE_NE_P1    (SPRITE_BUFFER + 0x30)
#define SPRITE_E_P0     (SPRITE_BUFFER + 0x40)
#define SPRITE_E_P1     (SPRITE_BUFFER + 0x50)
#define SPRITE_SE_P0    (SPRITE_BUFFER + 0x60)
#define SPRITE_SE_P1    (SPRITE_BUFFER + 0x70)
#define SPRITE_S_P0     (SPRITE_BUFFER + 0x80)
#define SPRITE_S_P1     (SPRITE_BUFFER + 0x90)
#define SPRITE_SW_P0    (SPRITE_BUFFER + 0xA0)
#define SPRITE_SW_P1    (SPRITE_BUFFER + 0xB0)
#define SPRITE_W_P0     (SPRITE_BUFFER + 0xC0)
#define SPRITE_W_P1     (SPRITE_BUFFER + 0xD0)
#define SPRITE_NW_P0    (SPRITE_BUFFER + 0xE0)
#define SPRITE_NW_P1    (SPRITE_BUFFER + 0xF0)


#define HSCROL  (*(unsigned char*)0xD404)  // Horizontal fine scroll register
#define VCOUNT  (*(volatile unsigned char*)0xD40B)  // Current scanline

#define PMG_MEM     0x3000
#define PLAYER0_GFX (PMG_MEM + 0x200)
#define PLAYER1_GFX (PMG_MEM + 0x280)

#define PORTA (*(unsigned char*)0xD300)
#define TRIG1 (*(unsigned char*)0xD010)  // 0 = pressed
#define POTGO (*(unsigned char*)0xD20B)
#define POT1 (*(unsigned char*)0x0270)

// Hardware registers
#define HPOSP0 53248    // Horizontal position of player 0
#define HPOSP1 53249    // Horizontal position of player 1 
#define SIZEP0 53256    // Size register for player 0 (0=normal, 1=double, 3=quadruple)
#define SIZEP1 53257    // Size register for player 1
#define GRACTL 53277    // Graphics control
#define DMACTL 559      // DMA control
#define PMBASE 54279    // Player/Missile base address
#define COLOR0 704      // Color for player 0
#define COLOR1 705      // Color for player 1
#define STICK0 632      // Joystick 0 value


#define SCREEN_HEIGHT 24
#define SCREEN_WIDTH  40
#define MAP_WIDTH     200
#define MAP_HEIGHT    100

#define SPRITE_CENTER_Y 52
#define SPRITE_CENTER_X 20
unsigned char* charmap;
unsigned char* screen;
unsigned char scroll_offset = 0;
unsigned char pending_scroll_reset = 0;
unsigned char camera_x = 0;
unsigned char camera_y = 0;

unsigned char sprite_x = 20;  // screen-relative X position
unsigned char sprite_y = 52;  // screen-relative Y position
#define SCORE_ADDR 0xBEF0
#define GAS_ADDR 0xBF00

#define score (*(unsigned char*)SCORE_ADDR)
#define gas   (*(unsigned char*)GAS_ADDR)
#define prev (*(unsigned char*)0xBFB2)

#define orientation (*(volatile unsigned char*)0x0400)

#define scroll_hold_counter (*(unsigned char*)0xBDA0)

#define scroll_speed (*(unsigned char*)0xBDA1)
#define gas_counter  (*(unsigned char*)0xBDA2)
#define STUN_TIMER_ADDR 0xBCC0
#define stun_timer (*(volatile unsigned char*)STUN_TIMER_ADDR)
#define CHECKPOINTS_H_ADDR 0xBEA0


/* Orientation indices */
#define ORIENT_N   0
#define ORIENT_NE  1
#define ORIENT_E   2
#define ORIENT_SE  3
#define ORIENT_S   4
#define ORIENT_SW  5
#define ORIENT_W   6
#define ORIENT_NW  7


// Lookup table to convert sprite_x tile to hpos value
unsigned char* tile_to_hpos = (unsigned char*)SPRITE_TABLE_ADDR; 


/* --- N (Up) ---------------------------------------------------------- */
const unsigned char car_N_p0[16] = {
    0b00000111,
    0b00001100,
    0b00111110,
    0b01110110,
    0b01001111,
    0b01110000,
    0b01111000,
    0b00111111,
    0b00010111,
    0b00010111,
    0b00111011,
    0b01111000,
    0b01111100,
    0b01111111,
    0b00100111,
    0b00011100,
};

const unsigned char car_N_p1[16] = {
    0b11100000,
    0b00110000,
    0b01111100,
    0b01101110,
    0b11110010,
    0b00001110,
    0b00011110,
    0b11111100,
    0b11101000,
    0b11101000,
    0b11011100,
    0b00011110,
    0b00111110,
    0b11111110,
    0b11100100,
    0b00111000,
};
/* --- NE (provisional = N OR E mix; edit!) ---------------------------- */
const unsigned char car_NE_p0[16] = {
    0b00000001,
    0b00000011, 
    0b00000111, 
    0b00000110,
    0b00000111,
    0b00001111,
    0b01111101, 
    0b11111011,
    0b11111011,  
    0b11011001,  
    0b01101100,  
    0b01111111, 
    0b00011011, 
    0b00001101, 
    0b00001111, 
    0b00000011, 

};
const unsigned char car_NE_p1[16] = {
    0b11000000,
    0b01111000,
    0b10101100,
    0b01110110,
    0b00101010,
    0b00011110,
    0b10001011,
    0b11000101,
    0b11110111,
    0b10111110,
    0b01111100,
    0b11100000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b10000000,

};

/* --- E (Right) ------------------------------------------------------- */
const unsigned char car_E_p0[16] = {
    0b00000000,
    0b00111000,
    0b01111100,
    0b10111111,
    0b10111100,
    0b11110011,
    0b01100111,
    0b01100111,
    0b01100111,
    0b01100111,
    0b11110011,
    0b10111100,
    0b10111111,
    0b01111100,
    0b00111000,
    0b00000000,
};
const unsigned char car_E_p1[16] = {
    0b00000000,
    0b01111000,
    0b11101100,
    0b11101100,
    0b11010110,
    0b10011111,
    0b10011101,
    0b10010001,
    0b10010001,
    0b10011101,
    0b10011111,
    0b11010110,
    0b11101100,
    0b11101100,
    0b01111000,
    0b00000000,

};

/* --- SE (provisional = S OR E mix; edit!) ---------------------------- */
const unsigned char car_SE_p0[16] = {
    0b00000011,
    0b00001111,
    0b00001101,
    0b00011011,
    0b01111111,
    0b01101100,
    0b11011001,
    0b11111011,
    0b11111011,
    0b01111101,
    0b00001111,
    0b00000111,
    0b00000110,
    0b00000111,
    0b00000011,
    0b00000001,
};

const unsigned char car_SE_p1[16] = {
    0b10000000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b11100000,
    0b01111100,
    0b10111110,
    0b11110111,
    0b11000101,
    0b10001011,
    0b00011110,
    0b00101010,
    0b01110110,
    0b10101100,
    0b01111000,
    0b11000000,
};


/* --- S (Down) -------------------------------------------------------- */
const unsigned char car_S_p0[16] = {
    0b00011100,
    0b00100111,
    0b01111111,
    0b01111100,
    0b01111000,
    0b00111011,
    0b00010111,
    0b00010111,
    0b00111111,
    0b01110000,
    0b01111000,
    0b01001111,
    0b01110110,
    0b00111110,
    0b00001100,
    0b00000111,

};
const unsigned char car_S_p1[16] = {
    0b00111000,
    0b11100100,
    0b11111110,
    0b00111110,
    0b00011110,
    0b11011100,
    0b11101000,
    0b11101000,
    0b11111100,
    0b00001110,
    0b00011110,
    0b11110010,
    0b01101110,
    0b01111100,
    0b00110000,
    0b11100000,

};

/* --- SW (provisional = S OR W mix; edit!) ---------------------------- */
const unsigned char car_SW_p0[16] = {
    0b00000001,
    0b00000011,
    0b00000011,
    0b00000011,
    0b00000111,
    0b00111110,
    0b01111101,
    0b11101111,
    0b10100011,
    0b11010001,
    0b01111000,
    0b01010100,
    0b01101110,
    0b00110101,
    0b00011110,
    0b00000011,
};

const unsigned char car_SW_p1[16] = {
    0b11000000,
    0b11110000,
    0b10110000,
    0b11011000,
    0b11111110,
    0b00110110,
    0b10011011,
    0b11011111,
    0b11011111,
    0b10111110,
    0b11110000,
    0b11100000,
    0b01100000,
    0b11100000,
    0b11000000,
    0b10000000,
};

/* --- W (Left) -------------------------------------------------------- */
const unsigned char car_W_p0[16] = {
    0b00000000,
    0b00011110,
    0b00110111,
    0b00110111,
    0b01101011,
    0b11111001,
    0b10111001,
    0b10001001,
    0b10001001,
    0b10111001,
    0b11111001,
    0b01101011,
    0b00110111,
    0b00110111,
    0b00011110,
    0b00000000,
    
};


const unsigned char car_W_p1[16] = {
    0b00000000,
    0b00011100,
    0b00111110,
    0b11111101,
    0b00111101,
    0b11001111,
    0b11100110,
    0b11100110,
    0b11100110,
    0b11100110,
    0b11001111,
    0b00111101,
    0b11111101,
    0b00111110,
    0b00011100,
    0b00000000,
};



/* --- NW (provisional = N OR W mix; edit!) ---------------------------- */
const unsigned char car_NW_p0[16] = {
    0b00000011,
    0b00011110,
    0b00110101,
    0b01101110,
    0b01010100,
    0b01111000,
    0b11010001,
    0b10100011,
    0b11101111,
    0b01111101,
    0b00111110,
    0b00000111,
    0b00000011,
    0b00000011,
    0b00000011,
    0b00000001,
};

const unsigned char car_NW_p1[16] = {
    0b10000000,
    0b11000000,
    0b11100000,
    0b01100000,
    0b11100000,
    0b11110000,
    0b10111110,
    0b11011111,
    0b11011111,
    0b10011011,
    0b00110110,
    0b11111110,
    0b11011000,
    0b10110000,
    0b11110000,
    0b11000000,
};
void clear_players(void) {
    // Use memset for efficiency
    memset((void*)(PLAYER0_GFX), 0, 256);
    memset((void*)(PLAYER1_GFX), 0, 256);
}

// Loads the appropriate sprite
static void load16(const unsigned char* src0, const unsigned char* src1)
{
    unsigned char i;
    for(i=0;i<16;++i){
        POKE(PLAYER0_GFX + sprite_y + i, src0[i]);
        POKE(PLAYER1_GFX + sprite_y + i, src1[i]);
    }
    POKE(HPOSP0, tile_to_hpos[sprite_x]); /* keep right half aligned */
    POKE(HPOSP1, tile_to_hpos[sprite_x + 2]); /* keep right half aligned */
}

// Loads the sprite based on the orientation
static void load_car_sprite(unsigned char o)
{
    switch(o & 7){
        case ORIENT_N:  load16(SPRITE_N_P0,  SPRITE_N_P1);  break;
        case ORIENT_NE: load16(SPRITE_NE_P0, SPRITE_NE_P1); break;
        case ORIENT_E:  load16(SPRITE_E_P0,  SPRITE_E_P1);  break;
        case ORIENT_SE: load16(SPRITE_SE_P0, SPRITE_SE_P1); break;
        case ORIENT_S:  load16(SPRITE_S_P0,  SPRITE_S_P1);  break;
        case ORIENT_SW: load16(SPRITE_SW_P0, SPRITE_SW_P1); break;
        case ORIENT_W:  load16(SPRITE_W_P0,  SPRITE_W_P1);  break;
        case ORIENT_NW: load16(SPRITE_NW_P0, SPRITE_NW_P1); break;
    }
}

// Checks if column that car is approaching is not blocking
bool is_walkable_column(int x, int y_start, int y_end) {
    unsigned char y;
    for (y = y_start; y <= y_end; y++) {
        if (x < 0 || y < 0) return false;
        if (charmap[y * MAP_WIDTH + x] == 1) return false;
    }
    return true;
}

// Checks if row that car is approaching is not blocking
bool is_walkable_row(int y, int x_start, int x_end) {
    unsigned char x;
    for (x = x_start; x < x_end; x++) {
        if (x >= MAP_WIDTH || y >= MAP_HEIGHT) return false;
        if (charmap[y * MAP_WIDTH + x] == 1) return false;
    }
    return true;
}

// Builds the character map of the road outline
void characterMap()
{
    int i;
    POKE(709, 15);

    
    // square 1x1
    for (i = 14; i < 104; i += 2)
    {
        charmap[2 * MAP_WIDTH + i] = 1;
    }
    charmap[3 * MAP_WIDTH + 13] = 1;
    charmap[3 * MAP_WIDTH + 12] = 1;
    charmap[3 * MAP_WIDTH + 11] = 1;

    charmap[4 * MAP_WIDTH + 10] = 1;
    charmap[4 * MAP_WIDTH + 9] = 1;

    charmap[5 * MAP_WIDTH + 8] = 1;
    charmap[6 * MAP_WIDTH + 7] = 1;
    charmap[7 * MAP_WIDTH + 7] = 1;

    charmap[8 * MAP_WIDTH + 6] = 1;
    charmap[9 * MAP_WIDTH + 6] = 1;
    charmap[10 * MAP_WIDTH + 6] = 1;

    //char 2x1 and 3x1
    for (i = 11; i < 50; i++)
    {
        charmap[i * MAP_WIDTH + 5] = 1;
    }
    
    charmap[50 * MAP_WIDTH + 6] = 1;
    charmap[51 * MAP_WIDTH + 6] = 1;
    charmap[52 * MAP_WIDTH + 6] = 1;

    charmap[53 * MAP_WIDTH + 7] = 1;
    charmap[54 * MAP_WIDTH + 7] = 1;

    charmap[55 * MAP_WIDTH + 8] = 1;

    charmap[56 * MAP_WIDTH + 9] = 1;
    charmap[56 * MAP_WIDTH + 10] = 1;
    
    charmap[57 * MAP_WIDTH + 11] = 1;
    charmap[57 * MAP_WIDTH + 12] = 1;
    charmap[57 * MAP_WIDTH + 13] = 1;

    for (i = 30; i < 86; i += 2)
    {
        charmap[15 * MAP_WIDTH + i] = 1;
    }
    charmap[16 * MAP_WIDTH + 29] = 1;
    charmap[16 * MAP_WIDTH + 28] = 1;

    charmap[17 * MAP_WIDTH + 27] = 1;

    charmap[18 * MAP_WIDTH + 26] = 1;
    charmap[19 * MAP_WIDTH + 26] = 1;


    for (i = 20; i < 42; i++)
    {
        charmap[i * MAP_WIDTH + 25] = 1;
    }


    charmap[42 * MAP_WIDTH + 26] = 1;
    charmap[43 * MAP_WIDTH + 26] = 1;

    charmap[44 * MAP_WIDTH + 27] = 1;

    charmap[45 * MAP_WIDTH + 28] = 1;
    charmap[45 * MAP_WIDTH + 29] = 1;

    for (i = 30; i < 71; i += 2)
    {
        charmap[46 * MAP_WIDTH + i] = 1;
    }

    for (i = 14; i < 55; i += 2)
    {
        charmap[58 * MAP_WIDTH + i] = 1;
    }

    // square 3x2

    charmap[47 * MAP_WIDTH + 71] = 1;
    charmap[47 * MAP_WIDTH + 72] = 1;
    charmap[47 * MAP_WIDTH + 73] = 1;

    charmap[48 * MAP_WIDTH + 74] = 1;
    charmap[48 * MAP_WIDTH + 75] = 1;

    charmap[49 * MAP_WIDTH + 76] = 1;

    charmap[50 * MAP_WIDTH + 77] = 1;
    charmap[51 * MAP_WIDTH + 77] = 1;

    charmap[52 * MAP_WIDTH + 78] = 1;
    charmap[53 * MAP_WIDTH + 78] = 1;
    charmap[54 * MAP_WIDTH + 78] = 1;

    for (i = 55; i < 70; i++)
    {
        charmap[i * MAP_WIDTH + 79] = 1;
    }

    charmap[59 * MAP_WIDTH + 55] = 1;
    charmap[59 * MAP_WIDTH + 56] = 1;

    charmap[60 * MAP_WIDTH + 57] = 1;

    charmap[61 * MAP_WIDTH + 58] = 1;
    charmap[62 * MAP_WIDTH + 58] = 1;

    charmap[63* MAP_WIDTH + 57] = 1;
    charmap[64* MAP_WIDTH + 57] = 1;

    charmap[65* MAP_WIDTH + 56] = 1;

    charmap[66* MAP_WIDTH + 55] = 1;
    charmap[66* MAP_WIDTH + 54] = 1;

    for (i = 53; i > 8; i -= 2)
    {
        charmap[67 * MAP_WIDTH + i] = 1;
    }

    charmap[70* MAP_WIDTH + 78] = 1;
    charmap[71* MAP_WIDTH + 78] = 1;
    charmap[72* MAP_WIDTH + 78] = 1;

    charmap[73* MAP_WIDTH + 77] = 1;
    charmap[74* MAP_WIDTH + 77] = 1;

    charmap[75* MAP_WIDTH + 76] = 1;

    charmap[76* MAP_WIDTH + 75] = 1;
    charmap[76* MAP_WIDTH + 74] = 1;

    charmap[77* MAP_WIDTH + 73] = 1;
    charmap[77* MAP_WIDTH + 72] = 1;
    charmap[77* MAP_WIDTH + 71] = 1;

    for (i = 69; i > 25; i -= 2)
    {
        charmap[78 * MAP_WIDTH + i] = 1;
    }


    charmap[68* MAP_WIDTH + 8] = 1;
    charmap[68* MAP_WIDTH + 7] = 1;
    charmap[68* MAP_WIDTH + 6] = 1;

    charmap[69* MAP_WIDTH + 5] = 1;
    charmap[69* MAP_WIDTH + 4] = 1;

    charmap[70* MAP_WIDTH + 3] = 1;

    charmap[71* MAP_WIDTH + 2] = 1;
    charmap[72* MAP_WIDTH + 2] = 1;

    charmap[73* MAP_WIDTH + 1] = 1;
    charmap[74* MAP_WIDTH + 1] = 1;
    charmap[75* MAP_WIDTH + 1] = 1;

    for(i = 76; i < 91; i++)
    {
        charmap[i * MAP_WIDTH + 0] = 1;
    }

    charmap[79* MAP_WIDTH + 26] = 1;
    charmap[79* MAP_WIDTH + 25] = 1;
    
    charmap[80* MAP_WIDTH + 24] = 1;

    charmap[81* MAP_WIDTH + 23] = 1;
    charmap[82* MAP_WIDTH + 23] = 1;
    charmap[83* MAP_WIDTH + 23] = 1;

    charmap[84* MAP_WIDTH + 24] = 1;

    charmap[85* MAP_WIDTH + 25] = 1;
    charmap[85* MAP_WIDTH + 26] = 1;

    for (i = 27; i < 90; i +=2)
    {
        charmap[86* MAP_WIDTH + i] = 1;
    } 

    charmap[91* MAP_WIDTH + 1] = 1;
    charmap[92* MAP_WIDTH + 1] = 1;
    charmap[93* MAP_WIDTH + 1] = 1;

    charmap[94* MAP_WIDTH + 2] = 1;
    charmap[95* MAP_WIDTH + 2] = 1;
    charmap[96* MAP_WIDTH + 3] = 1;

    charmap[97* MAP_WIDTH + 4] = 1;
    charmap[97* MAP_WIDTH + 5] = 1;
    
    charmap[98* MAP_WIDTH + 6] = 1;
    charmap[98* MAP_WIDTH + 7] = 1;
    charmap[98* MAP_WIDTH + 8] = 1;

    for (i = 9; i < 104; i+= 2)
    {
        charmap[99* MAP_WIDTH + i] = 1;
    }


    charmap[98* MAP_WIDTH + 104] = 1;
    charmap[98* MAP_WIDTH + 105] = 1;
    charmap[98* MAP_WIDTH + 106] = 1;

    charmap[97* MAP_WIDTH + 107] = 1;
    charmap[97* MAP_WIDTH + 108] = 1;

    charmap[96* MAP_WIDTH + 109] = 1;

    charmap[95* MAP_WIDTH + 110] = 1;
    charmap[94* MAP_WIDTH + 110] = 1;

    charmap[93* MAP_WIDTH + 111] = 1;
    charmap[92* MAP_WIDTH + 111] = 1;
    charmap[91* MAP_WIDTH + 111] = 1;


    charmap[85* MAP_WIDTH + 90] = 1;
    charmap[85* MAP_WIDTH + 91] = 1;
    charmap[85* MAP_WIDTH + 92] = 1;

    charmap[84* MAP_WIDTH + 93] = 1;
    charmap[84* MAP_WIDTH + 94] = 1;

    charmap[83* MAP_WIDTH + 95] = 1;

    charmap[82* MAP_WIDTH + 96] = 1;
    charmap[81* MAP_WIDTH + 96] = 1;

    charmap[80* MAP_WIDTH + 97] = 1;
    charmap[79* MAP_WIDTH + 97] = 1;
    charmap[78* MAP_WIDTH + 97] = 1;


    for (i = 90; i > 55; i--)
    {
        charmap[i * MAP_WIDTH + 112] = 1;
    }

    for (i = 77; i > 50; i--)
    {
        charmap[i* MAP_WIDTH + 98] = 1;
    }

    charmap[55 * MAP_WIDTH + 113] = 1;
    charmap[54 * MAP_WIDTH + 113] = 1;

    charmap[53 * MAP_WIDTH + 114] = 1;

    charmap[52 * MAP_WIDTH + 115] = 1;
    charmap[52 * MAP_WIDTH + 116] = 1;
    charmap[52 * MAP_WIDTH + 117] = 1;

    charmap[53 * MAP_WIDTH + 118] = 1;

    charmap[54 * MAP_WIDTH + 119] = 1;
    charmap[55 * MAP_WIDTH + 119] = 1;

    for (i = 56; i < 91; i++)
    {
        charmap[i * MAP_WIDTH + 120] = 1;
    }

    charmap[50 * MAP_WIDTH + 99] = 1;
    charmap[49 * MAP_WIDTH + 99] = 1;
    charmap[48 * MAP_WIDTH + 99] = 1;

    charmap[47 * MAP_WIDTH + 100] = 1;
    charmap[46 * MAP_WIDTH + 100] = 1;

    charmap[45 * MAP_WIDTH + 101] = 1;

    charmap[44 * MAP_WIDTH + 102] = 1;
    charmap[44 * MAP_WIDTH + 103] = 1;

    charmap[43 * MAP_WIDTH + 104] = 1;
    charmap[43 * MAP_WIDTH + 105] = 1;
    charmap[43 * MAP_WIDTH + 106] = 1;


    for (i = 107; i < 130; i += 2)
    {
        charmap[42 * MAP_WIDTH + i] = 1;
    }

    charmap[43 * MAP_WIDTH + 130] = 1;
    charmap[43 * MAP_WIDTH + 131] = 1;
    charmap[43 * MAP_WIDTH + 132] = 1;

    charmap[44 * MAP_WIDTH + 133] = 1;
    charmap[44 * MAP_WIDTH + 134] = 1;

    charmap[45 * MAP_WIDTH + 135] = 1;

    charmap[46 * MAP_WIDTH + 136] = 1;
    charmap[47 * MAP_WIDTH + 136] = 1;
    
    charmap[48 * MAP_WIDTH + 137] = 1;
    charmap[49 * MAP_WIDTH + 137] = 1;
    charmap[50 * MAP_WIDTH + 137] = 1;

    for (i = 51; i < 81; i++)
    {
        charmap[i * MAP_WIDTH + 138] = 1;
    }

    charmap[91 * MAP_WIDTH + 121] = 1;
    charmap[92 * MAP_WIDTH + 121] = 1;
    charmap[93 * MAP_WIDTH + 121] = 1;

    charmap[94 * MAP_WIDTH + 122] = 1;
    charmap[95 * MAP_WIDTH + 122] = 1;

    charmap[96 * MAP_WIDTH + 123] = 1;

    charmap[97 * MAP_WIDTH + 124] = 1;
    charmap[97 * MAP_WIDTH + 125] = 1;

    charmap[98 * MAP_WIDTH + 126] = 1;
    charmap[98 * MAP_WIDTH + 127] = 1;
    charmap[98 * MAP_WIDTH + 128] = 1;
    
    charmap[81 * MAP_WIDTH + 139] = 1;
    charmap[82 * MAP_WIDTH + 139] = 1;

    charmap[83 * MAP_WIDTH + 140] = 1;

    charmap[84 * MAP_WIDTH + 141] = 1;
    charmap[84 * MAP_WIDTH + 142] = 1;

    for (i = 129; i < 186; i+=2)
    {
        charmap[99 * MAP_WIDTH + i] = 1;
    }

    for (i = 143; i < 170; i+=2)
    {
        charmap[85 * MAP_WIDTH + i] = 1;
    }
    
    charmap[98 * MAP_WIDTH + 186] = 1;
    charmap[98 * MAP_WIDTH + 187] = 1;
    charmap[98 * MAP_WIDTH + 188] = 1;

    charmap[97 * MAP_WIDTH + 189] = 1;
    charmap[97 * MAP_WIDTH + 190] = 1;

    charmap[96 * MAP_WIDTH + 191] = 1;

    charmap[95 * MAP_WIDTH + 192] = 1;
    charmap[94 * MAP_WIDTH + 192] = 1;
    
    charmap[93 * MAP_WIDTH + 193] = 1;
    charmap[92 * MAP_WIDTH + 193] = 1;
    charmap[91 * MAP_WIDTH + 193] = 1;

    charmap[84 * MAP_WIDTH + 170] = 1;
    charmap[84 * MAP_WIDTH + 171] = 1;

    charmap[83 * MAP_WIDTH + 172] = 1;

    charmap[82 * MAP_WIDTH + 173] = 1;
    charmap[81 * MAP_WIDTH + 173] = 1;

    for (i = 80; i > 19; i -= 2)
    {
        charmap[i * MAP_WIDTH + 174] = 1;
    }

    for (i = 90; i > 10; i -= 2)
    {
        charmap[i * MAP_WIDTH + 194] = 1;
    }

    charmap[10 * MAP_WIDTH + 193] = 1;
    charmap[9 * MAP_WIDTH + 193] = 1;
    charmap[8 * MAP_WIDTH + 193] = 1;

    charmap[7 * MAP_WIDTH + 192] = 1;
    charmap[6 * MAP_WIDTH + 192] = 1;

    charmap[5 * MAP_WIDTH + 191] = 1;

    charmap[4 * MAP_WIDTH + 190] = 1;
    charmap[4 * MAP_WIDTH + 189] = 1;

    charmap[3 * MAP_WIDTH + 188] = 1;
    charmap[3 * MAP_WIDTH + 187] = 1;
    charmap[3 * MAP_WIDTH + 186] = 1;

    for (i = 185; i > 138; i -= 2)
    {
        charmap[2 * MAP_WIDTH + i] = 1;
    }

    charmap[19 * MAP_WIDTH + 173] = 1;
    charmap[18 * MAP_WIDTH + 173] = 1;

    charmap[17 * MAP_WIDTH + 172] = 1;

    charmap[16 * MAP_WIDTH + 171] = 1;
    charmap[16 * MAP_WIDTH + 170] = 1;

    for (i = 169; i > 155; i -= 2)
    {
        charmap[15 * MAP_WIDTH + i] = 1;
    }
    
    charmap[16 * MAP_WIDTH + 156] = 1;
    charmap[16 * MAP_WIDTH + 155] = 1;

    charmap[17 * MAP_WIDTH + 154] = 1;

    charmap[18 * MAP_WIDTH + 153] = 1;
    charmap[19 * MAP_WIDTH + 153] = 1;

    charmap[3 * MAP_WIDTH + 137] = 1;
    charmap[3 * MAP_WIDTH + 136] = 1;
    charmap[3 * MAP_WIDTH + 135] = 1;

    charmap[4 * MAP_WIDTH + 134] = 1;
    charmap[4 * MAP_WIDTH + 133] = 1;

    charmap[5 * MAP_WIDTH + 132] = 1;

    charmap[6 * MAP_WIDTH + 131] = 1;
    charmap[7 * MAP_WIDTH + 131] = 1;

    charmap[8 * MAP_WIDTH + 130] = 1;
    charmap[9 * MAP_WIDTH + 130] = 1;
    charmap[10 * MAP_WIDTH + 130] = 1;

    for (i = 11; i < 22; i++)
    {
        charmap[i * MAP_WIDTH + 129] = 1;
    }
    
    charmap[22 * MAP_WIDTH + 128] = 1;
    charmap[23 * MAP_WIDTH + 128] = 1;

    charmap[24 * MAP_WIDTH + 127] = 1;

    charmap[25 * MAP_WIDTH + 126] = 1;
    charmap[25 * MAP_WIDTH + 125] = 1;

    for (i = 20; i < 31; i++)
    {
        charmap[i * MAP_WIDTH + 152] = 1;
    }
    charmap[25 * MAP_WIDTH + 125] = 1;
    
    charmap[31 * MAP_WIDTH + 151] = 1;
    charmap[32 * MAP_WIDTH + 151] = 1;
    charmap[33 * MAP_WIDTH + 151] = 1;

    charmap[34 * MAP_WIDTH + 150] = 1;
    charmap[35 * MAP_WIDTH + 150] = 1;

    charmap[36 * MAP_WIDTH + 149] = 1;

    charmap[37 * MAP_WIDTH + 148] = 1;
    charmap[37 * MAP_WIDTH + 147] = 1;

    charmap[38 * MAP_WIDTH + 146] = 1;
    charmap[38 * MAP_WIDTH + 145] = 1;
    charmap[38 * MAP_WIDTH + 144] = 1;

    for (i = 143; i > 98; i-= 2)
    {
        charmap[39 * MAP_WIDTH + i] = 1;
    }
    charmap[38 * MAP_WIDTH + 98] = 1;
    charmap[38 * MAP_WIDTH + 97] = 1;
    charmap[38 * MAP_WIDTH + 96] = 1;
    
    charmap[37 * MAP_WIDTH + 95] = 1;
    charmap[37 * MAP_WIDTH + 94] = 1;
    
    charmap[36 * MAP_WIDTH + 93] = 1;

    charmap[35 * MAP_WIDTH + 92] = 1;
    charmap[34 * MAP_WIDTH + 92] = 1;
    
    charmap[33 * MAP_WIDTH + 91] = 1;
    charmap[32 * MAP_WIDTH + 91] = 1;
    charmap[31 * MAP_WIDTH + 91] = 1;

    for (i = 124; i > 115; i-= 2)
    {
        charmap[26 * MAP_WIDTH + i] = 1;
    }

    charmap[25 * MAP_WIDTH + 115] = 1;
    charmap[25 * MAP_WIDTH + 114] = 1;

    charmap[24 * MAP_WIDTH + 113] = 1;
    
    charmap[23 * MAP_WIDTH + 112] = 1;
    charmap[22 * MAP_WIDTH + 112] = 1;


    for (i = 30; i > 19; i--)
    {
        charmap[i * MAP_WIDTH + 90] = 1;
    }

    charmap[19 * MAP_WIDTH + 89] = 1;
    charmap[18 * MAP_WIDTH + 89] = 1;
  
    for (i = 21; i > 10; i--)
    {
        charmap[i * MAP_WIDTH + 111] = 1;
    }
    charmap[17 * MAP_WIDTH + 88] = 1;

    charmap[16 * MAP_WIDTH + 87] = 1;
    charmap[16 * MAP_WIDTH + 86] = 1;
    

    charmap[10 * MAP_WIDTH + 110] = 1;
    charmap[9 * MAP_WIDTH + 110] = 1;
    charmap[8 * MAP_WIDTH + 110] = 1;
    
    charmap[7 * MAP_WIDTH + 109] = 1;
    charmap[6 * MAP_WIDTH + 109] = 1;
    
    charmap[5 * MAP_WIDTH + 108] = 1;

    charmap[4 * MAP_WIDTH + 107] = 1;
    charmap[4 * MAP_WIDTH + 106] = 1;
    
    charmap[3 * MAP_WIDTH + 105] = 1;
    charmap[3 * MAP_WIDTH + 104] = 1;
    charmap[3 * MAP_WIDTH + 103] = 1;
}

// Sets up the display list, one text line, 23 lines of game
void install_display_list() {

    unsigned char* dl = (unsigned char*)DISPLAY_LIST;
    unsigned int screen_address = (unsigned int)SCREEN_ADDR;
    int i, j = 0;

    // 3 blank scanlines (optional)
    dl[j++] = 0x70;
    dl[j++] = 0x70;
    dl[j++] = 0x70;

    dl[j++] = 0x42;  // 1000 0010 → Mode 2 + LMS
    dl[j++] = (screen_address & 0xFF);
    dl[j++] = (screen_address >> 8) & 0xFF;

    // 23 lines of ANTIC Mode 4 (Graphics 12)
    for (i = 1; i < SCREEN_HEIGHT; i++) {
        dl[j++] = 0x04;  // ANTIC Mode 4
    }

    // Jump to self (JVB)
    dl[j++] = 0x41;
    dl[j++] = ((unsigned int)dl) & 0xFF;
    dl[j++] = ((unsigned int)dl) >> 8;

    // Set DL pointer
    POKE(560, (unsigned int)dl & 0xFF);
    POKE(561, (unsigned int)dl >> 8);
}

// Draws the map by copying the entire desired row
// to the correct pointer in the screen pointer
void draw_map() {
    unsigned char* dst = screen + SCREEN_WIDTH;  // Start at row 1 (row 0 is text line)
    unsigned char* src = &charmap[camera_y * MAP_WIDTH + camera_x];
    int row;
    
    for (row = 0; row < SCREEN_HEIGHT - 1; row++) {
        memcpy(dst, src, SCREEN_WIDTH);
        dst += SCREEN_WIDTH;
        src += MAP_WIDTH;
    }
}

// Updates the text line of the screen with
// the correct gas and score values
void update_hud(unsigned int hud_score, unsigned int hud_gas) {

    unsigned int dis_score = hud_score;
    unsigned int dis_gas = hud_gas;

    HUD_BUFFER[0]  = 'S' - 0x20;
    HUD_BUFFER[1]  = 'C' - 0x20;
    HUD_BUFFER[2]  = 'O' - 0x20;
    HUD_BUFFER[3]  = 'R' - 0x20;
    HUD_BUFFER[4]  = 'E' - 0x20;

    HUD_BUFFER[7] = (dis_score / 10) % 10 + 0x10;
    HUD_BUFFER[8] = (dis_score % 10) + 0x10;
    HUD_BUFFER[9] = 0x10;

    HUD_BUFFER[26]  = 'F' - 0x20;
    HUD_BUFFER[27]  = 'U' - 0x20;
    HUD_BUFFER[28]  = 'E' - 0x20;
    HUD_BUFFER[29]  = 'L' - 0x20;

    HUD_BUFFER[31]  = dis_gas / 10 + 0x10;
    HUD_BUFFER[32] = (dis_gas % 10) + 0x10;

    // Copy to top row of screen
    memcpy(SCREEN_ADDR, HUD_BUFFER, 40);


}

unsigned char checkpoints_h[5] = {40, 60, 80, 0, 0};
unsigned char checkpoints_v[5] = {30, 60, 90, 120, 150};

// Checks the score, if the sprite is at one of the checkpoints
// Increment the score 
void check_score()
{
    unsigned char k; 
    for (k = 0; k < 5; k++){
        if (sprite_x + camera_x == checkpoints_v[k])
        {
            score++;
        }
    }
    
}

unsigned char tiles_scrolled; // Number of tiles scrolled
unsigned char blocked = 0; // 0 - not blocked, 1 - blocked
const unsigned char o_is_up[8] = { // Lookup table to tell if orientation is up
    1, 1, 0, 0, 0, 0, 0, 1
};
void update_scroll() {
    unsigned char o;
    unsigned char paddle;
    unsigned char accelerate = TRIG1;
    unsigned char state;
    int test_x, test_y, i;

    paddle = PORTA & 0x0F;
    o = orientation;    
    state = (prev << 4) | paddle;

    switch (state) {
        // CW
        case 0xFD: // 15 → 13
        case 0xDC: // 13 → 12
        case 0xCE: // 12 → 14
        case 0xEF: // 14 → 15
            orientation = (orientation + 1) & 7;
            prev = paddle;
            break;

        // CCW
        case 0xFE: // 15 → 14
        case 0xEC: // 14 → 12
        case 0xCD: // 12 → 13
        case 0xDF: // 13 → 15
            orientation = (orientation - 1) & 7;
            prev = paddle;
            break;

        default:
            prev = paddle;  // resync if unknown transition
            break;
    }

    gas_counter++;
        if (gas_counter == 40)
        {
            gas -= 1;
            gas_counter = 0;
            update_hud(score, gas);
        }
    
    if (accelerate == 0)
    {
        if (scroll_hold_counter < 25)
        {
            scroll_hold_counter++;
        }
    }
    else
    {
        if (scroll_hold_counter > 0)
        {
            scroll_hold_counter--;
        }
    }
    if (scroll_hold_counter > 24)
    {
        scroll_speed = 1;
    }
    else if (scroll_hold_counter > 16)
    {
        scroll_speed = 1;
    }
    else if (scroll_hold_counter > 8)
    {
        POKE(0xD202, 80);   // Set frequency
        POKE(0xD203, 0x21); // Set tone + volume
        scroll_speed = 1;
    }
    else
    {
        POKE(0xD203, 0x20); // Set tone + volume
        scroll_speed = 0;
    }

    while (VCOUNT > 8);
    while (VCOUNT <= 8);
    if(!blocked)
    {
 
        tiles_scrolled = scroll_speed;

            if (o >= 1 && o <= 3){ // EAST
                    //blocked = false;

                    for (i = 0; i <= scroll_speed; i++) {
                        test_x = camera_x + sprite_x + i + 2;
                        if (!is_walkable_column(test_x,
                                camera_y + (sprite_y / 4) - 4,
                                camera_y + (sprite_y / 4) - 2)) {
                            tiles_scrolled = i - 1;
                            blocked = 1;
                            break;
                        }
                    }

                    if (tiles_scrolled > 0) {

                        if (sprite_x < SPRITE_CENTER_X) {
                            if (sprite_x + tiles_scrolled > SPRITE_CENTER_X) {
                                sprite_x = SPRITE_CENTER_X;
                            } else {
                                sprite_x += tiles_scrolled;
                            }
                            memset((void*)PLAYER0_GFX, 0, 256);
                        } else if (camera_x < MAP_WIDTH - 42) {
                            camera_x += tiles_scrolled;
                            draw_map();
                        } else if (sprite_x + tiles_scrolled < 42) {
                            sprite_x += tiles_scrolled;
                            memset((void*)PLAYER0_GFX, 0, 256);
                        }
                        check_score();
                    }
                    
            }
        else if (o >= 5) // left
            { // LEFT
                // Check Collisions
                for (i = 1; i <= scroll_speed; i++) {
                    test_x = camera_x + sprite_x + 1- i;
                    if (!is_walkable_column(test_x,
                            camera_y + (sprite_y / 4) - 4,
                            camera_y + (sprite_y / 4) - 2)) {
                        tiles_scrolled = i - 1;
                        blocked = 1;
                        break;
                    }
                }
                // Scroll
                if (tiles_scrolled > 0) {
                    if (sprite_x > SPRITE_CENTER_X) { // If sprite is not centered
                        if (sprite_x - tiles_scrolled < SPRITE_CENTER_X) {
                            sprite_x = SPRITE_CENTER_X;
                        } else {
                            sprite_x -= tiles_scrolled;
                        }
                        memset((void*)PLAYER0_GFX, 0, 256);
                    } else if (camera_x > 0) { // If sprite is centered
                        camera_x -= tiles_scrolled;
                        draw_map();
                    } else if (sprite_x - tiles_scrolled > 0) { // If we are at the edge of the map
                        sprite_x -= tiles_scrolled;
                        memset((void*)PLAYER0_GFX, 0, 256);
                    }
                    check_score();
                }
                
            }

            if (o_is_up[o]) // up
            {
                //blocked = 0;

                for (i = 1; i <= scroll_speed; i++) {
                    test_y = camera_y + (sprite_y / 4) - 5 - i;
                    if (!is_walkable_row(test_y,
                            camera_x + (int)sprite_x - 1,
                            camera_x + (int)sprite_x + 1)) {
                        tiles_scrolled = i - 1;
                        blocked = 1;
                        break;
                    }
                }

                if (tiles_scrolled > 0) {
                    if (sprite_y > SPRITE_CENTER_Y) {
                        sprite_y -= tiles_scrolled * 4;
                        memset((void*)PLAYER0_GFX, 0, 256);
                    } else if (camera_y - tiles_scrolled >= 0) {
                        camera_y -= tiles_scrolled;
                        draw_map();
                    } else {
                        sprite_y -= tiles_scrolled * 4;
                        memset((void*)PLAYER0_GFX, 0, 256);
                    }
                }
                
            }
            else if (o >= 3 && o <= 5) // down
            {
                for (i = 1; i <= scroll_speed; i++) {
                    test_y = camera_y + (sprite_y / 4) - 2 + i;
                    if (!is_walkable_row(test_y,
                            camera_x + (int)sprite_x - 1,
                            camera_x + (int)sprite_x + 1)) {
                        tiles_scrolled = i - 1;
                        blocked = 1;
                        break;
                    }
                }

                if (tiles_scrolled > 0) {
                    if (sprite_y < SPRITE_CENTER_Y) {
                        sprite_y += tiles_scrolled * 4;
                        if (sprite_y > SPRITE_CENTER_Y) sprite_y = SPRITE_CENTER_Y;
                        memset((void*)PLAYER0_GFX, 0, 256);
                    } else if (camera_y + SCREEN_HEIGHT <= MAP_HEIGHT) {
                        camera_y += tiles_scrolled;
                        draw_map();
                    } else {
                        sprite_y += tiles_scrolled * 4;
                        memset((void*)PLAYER0_GFX, 0, 256);
                    }
                }
            }

        }
        else if (blocked)
        {
            //POKE(0xD203, 0x41); // Set tone + volume
            stun_timer++;
            if (stun_timer == 90)
            {
                blocked = 0;
                stun_timer = 0;
               
            }

        }

  load_car_sprite(orientation);
}

void copy_sprites_to_buffer() {
    memcpy(SPRITE_N_P0,    car_N_p0,    16);
    memcpy(SPRITE_N_P1,    car_N_p1,    16);
    memcpy(SPRITE_NE_P0,   car_NE_p0,   16);
    memcpy(SPRITE_NE_P1,   car_NE_p1,   16);
    memcpy(SPRITE_E_P0,    car_E_p0,    16);
    memcpy(SPRITE_E_P1,    car_E_p1,    16);
    memcpy(SPRITE_SE_P0,   car_SE_p0,   16);
    memcpy(SPRITE_SE_P1,   car_SE_p1,   16);
    memcpy(SPRITE_S_P0,    car_S_p0,    16);
    memcpy(SPRITE_S_P1,    car_S_p1,    16);
    memcpy(SPRITE_SW_P0,   car_SW_p0,   16);
    memcpy(SPRITE_SW_P1,   car_SW_p1,   16);
    memcpy(SPRITE_W_P0,    car_W_p0,    16);
    memcpy(SPRITE_W_P1,    car_W_p1,    16);
    memcpy(SPRITE_NW_P0,   car_NW_p0,   16);
    memcpy(SPRITE_NW_P1,   car_NW_p1,   16);
}

//static char score_buffer[11];  // Declare this outside of any function
void init_tile_to_hpos() {
    unsigned int i;
    for (i = 0; i < 40; i++) {
        tile_to_hpos[i] = 48 + (i * 4);  // Same as: 48, 52, ...
    }
}


void game_loop(void) {
    _graphics(13);  
    charmap = (unsigned char*)CHARMAP_ADDR;
    screen  = (unsigned char*)SCREEN_ADDR;

    gas = 40;
    score = 0;
    stun_timer = 0;
    prev = 15;
    orientation = 0;
    scroll_hold_counter = 0;
    gas_counter = 0;

    memset(charmap, 0, MAP_WIDTH * MAP_HEIGHT);
    memset(screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
    
    init_tile_to_hpos();
    copy_sprites_to_buffer();
    POKE(710, 0x00);  // black background
    POKE(623, 0);     // Clear ATASCII cursor

    memcpy(CUSTOM_CHARSET, (void*)0xE000, 1024);
    CUSTOM_CHARSET[1 * 8 + 0] = 0b10000010;
    CUSTOM_CHARSET[1 * 8 + 1] = 0b10000010;
    CUSTOM_CHARSET[1 * 8 + 2] = 0b00101000;
    CUSTOM_CHARSET[1 * 8 + 3] = 0b00101000;
    CUSTOM_CHARSET[1 * 8 + 4] = 0b00101000;
    CUSTOM_CHARSET[1 * 8 + 5] = 0b00101000;
    CUSTOM_CHARSET[1 * 8 + 6] = 0b10000010;
    CUSTOM_CHARSET[1 * 8 + 7] = 0b10000010;

    POKE(756, ((unsigned int)CUSTOM_CHARSET) >> 8);

    memset(screen, 0, SCREEN_WIDTH);
    install_display_list();
    characterMap();  
    draw_map();     

    memset((void*)PMG_MEM, 0, 1024);
    POKE(PMBASE, PMG_MEM >> 8);
    POKE(GRACTL, 0x03);
    POKE(DMACTL, 0x2E);
    POKE(COLOR0, 0xFE);
    POKE(COLOR1, 0xFE);
    POKE(SIZEP0, 0);
    POKE(SIZEP1, 0);
    update_hud(score, gas);
    POKE(HPOSP0, 60);

    while (1) {
        while(gas) {
            update_scroll();
            //POKE(0xD202, 120);   // Set frequency
        }
    }
        
}

void main(void) {
        // Install warm reset vector
    POKE(0x000A, (unsigned)&game_loop & 0xFF);
    POKE(0x000B, (unsigned)&game_loop >> 8);

    // Clean out DOS jump behavior
    POKE(0x02E0, 0);  // IOCB vector (optional)
    POKE(0x000C, 0);  // DOSINI (disable re-init)

    game_loop();  // Start the game
}
