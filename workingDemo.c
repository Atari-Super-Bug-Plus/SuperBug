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

#define CUSTOM_CHARSET ((unsigned char*)0xB000)         // Must be 1K aligned and not overlap with CHARMAP
#define CHARMAP_ADDR  0x6000
#define SCREEN_ADDR   ((unsigned char*)0xB400)
#define DISPLAY_LIST  0xBB00

#define HSCROL  (*(unsigned char*)0xD404)  // Horizontal fine scroll register
#define VCOUNT  (*(volatile unsigned char*)0xD40B)  // Current scanline

#define PMG_MEM     0x3000
#define PLAYER0_GFX (PMG_MEM + 0x200)
#define PLAYER1_GFX (PMG_MEM + 0x280)
#define POT1 (*(unsigned char*)0xD200)
#define TRIG1 (*(unsigned char*)0x027C)  // 0 = pressed

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

// Direction constants
#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3

#define TEXT_LINE      ((unsigned char*)SCREEN_ADDR)
#define TILEMAP_START  (SCREEN_ADDR + 40)


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
int row, col, i, j, x, y;
unsigned char scroll_speed_v = 1;
unsigned char scroll_speed_h = 1;
unsigned char scroll_hold_counter_v = 0;
unsigned char scroll_hold_counter_h = 0;
unsigned char sprite_x = 20;  // screen-relative X position
unsigned char sprite_y = 52;  // screen-relative Y position

#define ROTATE_COOLDOWN_FRAMES 10000  // Tune this value for slower/faster rotation
unsigned char rotate_cooldown = 0;

unsigned char xpos = 120;
unsigned char direction = DIR_UP;
unsigned char using_two_players = 0;  // Flag to track if we're using both players
unsigned char prev_direction = 255;
unsigned char last_steering = 0;  // 0 = neutral, 1 = left, 2 = right


// UP
unsigned char car_sprite_0[16] = {
    0b00111100,
    0b11111111,
    0b00011000,
    0b11011011,
    0b11111111,
    0b11011011,
    0b00111100,
    0b01100110,
    0b01000010,
    0b01000010,
    0b11000011,
    0b11000011,
    0b11100111,
    0b00111100,
    0b11000011,
    0b11111111
};

// Car sprite definition for down direction (8 pixels wide)
unsigned char car_sprite_180[16] = {
    0b11111111,
    0b11000011,
    0b00111100,
    0b11100111,
    0b11000011,
    0b11000011,
    0b01000010,
    0b01000010,
    0b01100110,
    0b00111100,
    0b11011011,
    0b11111111,
    0b11011011,
    0b00011000,
    0b11111111,
    0b00111100
};

// Right-facing car sprite - left half (player 0)
unsigned char car_sprite_right_p0[16] = {
    0b11011100,  // Right edge of left half
    0b11011111,
    0b10110000,
    0b10100000,
    0b10100000,
    0b10110000,
    0b11011111,
    0b11011100,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};

// Right-facing car sprite - right half (player 1)
unsigned char car_sprite_right_p1[16] = {
    0b00111010,  // Left edge of right half
    0b10111010,
    0b11010011,
    0b01111111,
    0b01111111,
    0b11010011, 
    0b10111010,
    0b00111010,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};

// Left-facing car sprite - left half (player 0)
unsigned char car_sprite_left_p0[16] = {
    0b01011100,  // Left edge of left half
    0b01011101,
    0b11001011,
    0b11111110,
    0b11111110,
    0b11001011,
    0b01011101,
    0b01011100,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};

// Left-facing car sprite - right half (player 1)
unsigned char car_sprite_left_p1[16] = {
    0b00111011,  // Right edge of right half
    0b11111011,
    0b00001101,
    0b00000101,
    0b00000101,
    0b00001101,
    0b11111011,
    0b00111011,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};

void clear_players(void) {
    // Use memset for efficiency
    memset((void*)(PLAYER0_GFX), 0, 256);
    memset((void*)(PLAYER1_GFX), 0, 256);
}

void load_sprite(unsigned char* sprite, unsigned char player) {
    unsigned int i;
    unsigned int base = (player == 0) ? PLAYER0_GFX : PLAYER1_GFX;
    for (i = 0; i < 16; i++) {
        POKE(base + sprite_y + i, sprite[i]);  // ← vertical position now uses sprite_y
    }

    if (player == 0) {
        for (i = 0; i < 16; i++) {
            POKE(PLAYER1_GFX + sprite_y + i, 0);  // Clear player 1 when using just player 0
        }
    }

    POKE(HPOSP0, ((sprite_x * 161) / 40) + 39);  // Horizontal position
}

void load_right_facing_sprite(void) {
    unsigned int i;
    for (i = 0; i < 16; i++) {
        POKE(PLAYER0_GFX + sprite_y + i, car_sprite_right_p0[i]);
        POKE(PLAYER1_GFX + sprite_y + i, car_sprite_right_p1[i]);
    }
    POKE(HPOSP0, ((sprite_x * 161) / 40) + 39);
    POKE(HPOSP1, ((sprite_x * 161) / 40) + 47);
}

void load_left_facing_sprite(void) {
    unsigned int i;
    for (i = 0; i < 16; i++) {
        POKE(PLAYER0_GFX + sprite_y + i, car_sprite_left_p0[i]);
        POKE(PLAYER1_GFX + sprite_y + i, car_sprite_left_p1[i]);
    }
    POKE(HPOSP0, ((sprite_x * 161) / 40) + 39);
    POKE(HPOSP1, ((sprite_x * 161) / 40) + 47);
}

bool is_walkable_column(int x, int y_start, int y_end) {
    unsigned char y;

    for (y = y_start; y <= y_end; y++) {
        if (x < 0 || y < 0) return false;
        if (charmap[y * MAP_WIDTH + x] == 1) return false;
    }
    return true;
}

bool is_walkable_row(int y, int x_start, int x_end) {
    int x;
    for (x = x_start; x < x_end; x++) {
        if (x >= MAP_WIDTH || y >= MAP_HEIGHT) return false;
        if (charmap[y * MAP_WIDTH + x] == 1) return false;
    }
    return true;
}

void characterMap()
{
    
    POKE(709, 15);

    CUSTOM_CHARSET[1 * 8 + 0] = 0b00000000;
    CUSTOM_CHARSET[1 * 8 + 1] = 0b00000000;
    CUSTOM_CHARSET[1 * 8 + 2] = 0b00000000;
    CUSTOM_CHARSET[1 * 8 + 3] = 0b00000000;
    CUSTOM_CHARSET[1 * 8 + 4] = 0b00001010;
    CUSTOM_CHARSET[1 * 8 + 5] = 0b00001010;
    CUSTOM_CHARSET[1 * 8 + 6] = 0b00001010;
    CUSTOM_CHARSET[1 * 8 + 7] = 0b00001010;

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

    for (i = 80; i > 19; i--)
    {
        charmap[i * MAP_WIDTH + 174] = 1;
    }

    for (i = 90; i > 10; i--)
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

int draw_row_index = 0;

void install_display_list() {
    unsigned char* dl = (unsigned char*)DISPLAY_LIST;
    unsigned int line_addr;
    j = 0;

    // 3 blank scanlines
    dl[j++] = 0x70;
    dl[j++] = 0x70;
    dl[j++] = 0x70;

    for (i = 0; i < SCREEN_HEIGHT; i++) {
        line_addr = SCREEN_ADDR + i * SCREEN_WIDTH;
        dl[j++] = 0x44;  // Mode 4 + LMS + fine scroll
        dl[j++] = line_addr & 0xFF;
        dl[j++] = (line_addr >> 8) & 0xFF;
    }

    dl[j++] = 0x41;  // JVB
    dl[j++] = ((unsigned int)dl) & 0xFF;
    dl[j++] = ((unsigned int)dl) >> 8;
    POKE(560, (unsigned int)DISPLAY_LIST & 0xFF);
    POKE(561, (unsigned int)DISPLAY_LIST >> 8);
}
/*
void install_display_list() {
    unsigned char* dl = (unsigned char*)DISPLAY_LIST;
    unsigned int line_addr;
    j = 0;

    // 3 blank scanlines for vertical blank safety
    dl[j++] = 0x70;
    dl[j++] = 0x70;
    dl[j++] = 0x70;

    // Row 0: text mode (ANTIC mode 2 + LMS)
    line_addr = (unsigned int)TEXT_LINE;
    dl[j++] = 0x42;  // ANTIC mode 2 + LMS
    dl[j++] = line_addr & 0xFF;
    dl[j++] = line_addr >> 8;

    // Rows 1–23: tilemap rows (ANTIC mode 4 + LMS)
    for (i = 0; i < SCREEN_HEIGHT - 1; i++) {
        line_addr = (unsigned int)(TILEMAP_START + i * SCREEN_WIDTH);
        dl[j++] = 0x44;  // ANTIC mode 4 + LMS
        dl[j++] = line_addr & 0xFF;
        dl[j++] = line_addr >> 8;
    }

    // End with Jump to DL start (JVB)
    dl[j++] = 0x41;
    dl[j++] = (unsigned int)(dl) & 0xFF;
    dl[j++] = (unsigned int)(dl) >> 8;
    POKE(560, (unsigned int)DISPLAY_LIST & 0xFF);
    POKE(561, (unsigned int)DISPLAY_LIST >> 8);
}
*/
void draw_map() {
    unsigned char* dst = screen;
    unsigned char* src = &charmap[camera_y * MAP_WIDTH + camera_x];

    for (row = 0; row < SCREEN_HEIGHT; row++) {
        memcpy(dst, src, SCREEN_WIDTH);  // copy 41 columns per row
        dst += SCREEN_WIDTH;
        src += MAP_WIDTH;
    }
}

unsigned char tiles_scrolled;

void update_scroll() {
    bool blocked;
    int test_x, test_y, i;
    unsigned char joy_state = joy_read(JOY_1);

    while (VCOUNT > 8);
    while (VCOUNT <= 8);

    // RIGHT
    if (JOY_RIGHT(joy_state)) {
        scroll_hold_counter_h++;
        if (scroll_hold_counter_h == 8) scroll_speed_h = 1;
        else if (scroll_hold_counter_h == 16) scroll_speed_h = 1;
        else if (scroll_hold_counter_h == 24) scroll_speed_h = 1;

        tiles_scrolled = scroll_speed_h;
        blocked = false;

        for (i = 0; i <= scroll_speed_h; i++) {
            test_x = camera_x + sprite_x + i + 1;
            if (!is_walkable_column(test_x,
                    camera_y + (sprite_y / 4) - 3,
                    camera_y + (sprite_y / 4) - 2)) {
                tiles_scrolled = i - 1;
                blocked = true;
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
            } else if (camera_x < MAP_WIDTH - 42) {
                camera_x += tiles_scrolled;
                draw_map();
                pending_scroll_reset = 1;
            } else if (sprite_x + tiles_scrolled < 42) {
                sprite_x += tiles_scrolled;
                memset((void*)PLAYER0_GFX, 0, 256);
                memset((void*)PLAYER1_GFX, 0, 256);
            }
        }
    }

    // LEFT
    else if (JOY_LEFT(joy_state)) {
        scroll_hold_counter_h++;
        if (scroll_hold_counter_h == 8) scroll_speed_h = 1;
        else if (scroll_hold_counter_h == 16) scroll_speed_h = 1;
        else if (scroll_hold_counter_h == 24) scroll_speed_h = 1;

        tiles_scrolled = scroll_speed_h;
        blocked = false;

        for (i = 1; i <= scroll_speed_h; i++) {
            test_x = camera_x + (int)sprite_x - 2 - i;
            if (!is_walkable_column(test_x,
                    camera_y + (sprite_y / 4) - 3,
                    camera_y + (sprite_y / 4) - 2)) {
                tiles_scrolled = i - 1;
                blocked = true;
                break;
            }
        }

        if (tiles_scrolled > 0) {
            if (sprite_x > SPRITE_CENTER_X) {
                if (sprite_x - tiles_scrolled < SPRITE_CENTER_X) {
                    sprite_x = SPRITE_CENTER_X;
                } else {
                    sprite_x -= tiles_scrolled;
                }
            } else if (camera_x > 0) {
                camera_x -= tiles_scrolled;
                draw_map();
                pending_scroll_reset = 1;
            } else if (sprite_x - tiles_scrolled > 0) {
                sprite_x -= tiles_scrolled;
                memset((void*)PLAYER0_GFX, 0, 256);
                memset((void*)PLAYER1_GFX, 0, 256);
            }
        }
    }

    // UP
    if (JOY_UP(joy_state)) {
        scroll_hold_counter_v++;
        if (scroll_hold_counter_v == 8) scroll_speed_v = 1;
        else if (scroll_hold_counter_v == 16) scroll_speed_v = 1;
        else if (scroll_hold_counter_v == 24) scroll_speed_v = 1;

        tiles_scrolled = scroll_speed_v;
        blocked = false;

        for (i = 1; i <= scroll_speed_v; i++) {
            test_y = camera_y + (sprite_y / 4) - 4 - i;
            if (!is_walkable_row(test_y,
                    camera_x + (int)sprite_x - 1,
                    camera_x + (int)sprite_x + 1)) {
                tiles_scrolled = i - 1;
                blocked = true;
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
                pending_scroll_reset = 1;
            } else {
                sprite_y -= tiles_scrolled * 4;
                memset((void*)PLAYER0_GFX, 0, 256);
            }
        }
    }

    // DOWN
    else if (JOY_DOWN(joy_state)) {
        scroll_hold_counter_v++;
        if (scroll_hold_counter_v == 8) scroll_speed_v = 1;
        else if (scroll_hold_counter_v == 16) scroll_speed_v = 1;
        else if (scroll_hold_counter_v == 24) scroll_speed_v = 1;

        tiles_scrolled = scroll_speed_v;
        blocked = false;

        for (i = 1; i <= scroll_speed_v; i++) {
            test_y = camera_y + (sprite_y / 4) - 1 + i;
            if (!is_walkable_row(test_y,
                    camera_x + (int)sprite_x - 1,
                    camera_x + (int)sprite_x + 1)) {
                tiles_scrolled = i - 1;
                blocked = true;
                break;
            }
        }

        if (tiles_scrolled > 0) {
            if (sprite_y < SPRITE_CENTER_Y) {
                memset((void*)PLAYER0_GFX, 0, 256);
                sprite_y += tiles_scrolled * 4;
                if (sprite_y > SPRITE_CENTER_Y) sprite_y = SPRITE_CENTER_Y;
            } else if (camera_y + SCREEN_HEIGHT < MAP_HEIGHT) {
                camera_y += tiles_scrolled;
                draw_map();
                pending_scroll_reset = 1;
            } else {
                sprite_y += tiles_scrolled * 4;
                memset((void*)PLAYER0_GFX, 0, 256);
            }
        }
    }

    // Sprite direction updates
    if (JOY_UP(joy_state)) {
        load_sprite(car_sprite_0, 0);
    } else if (JOY_DOWN(joy_state)) {
        load_sprite(car_sprite_180, 0);
    } else if (JOY_LEFT(joy_state)) {
        load_left_facing_sprite();
        using_two_players = 1;
    } else if (JOY_RIGHT(joy_state)) {
        load_right_facing_sprite();
        using_two_players = 1;
    }
}

void main(void) {
    _graphics(12);  // ANTIC mode 4
    charmap = (unsigned char*)CHARMAP_ADDR;
    screen = (unsigned char*)SCREEN_ADDR;

    memset(charmap, 0, MAP_WIDTH * MAP_HEIGHT);
    memset(screen, 0, 40 + SCREEN_WIDTH * SCREEN_HEIGHT);

    memcpy(CUSTOM_CHARSET, (void*)0xE000, 1024);  // Use system ROM charset
    POKE(756, ((unsigned int)CUSTOM_CHARSET) >> 8);

    POKE(559, 0x3E);  // DMA enable + fine scroll
    /*
    TEXT_LINE[0] = 'H' - 32;
    TEXT_LINE[1] = 'E' - 32;
    TEXT_LINE[2] = 'L' - 32;
    TEXT_LINE[3] = 'L' - 32;
    TEXT_LINE[4] = 'O' - 32;
    */
    install_display_list();
    characterMap();
    draw_map();

 
  

    joy_install(&joy_static_stddrv);

    // Clear PMG memory

    // Set PMG base address
    POKE(DMACTL, 0x2E);         // Enable normal DMA + players (no missiles)
    POKE(GRACTL, 0x03);         // Enable player graphics
    POKE(PMBASE, PMG_MEM >> 8); // PMBASE = 0x30
    POKE(COLOR0, 0xFE);         // Bright white player
    POKE(COLOR1, 0xFE);
    POKE(SIZEP0, 0);            // Normal size
    POKE(SIZEP1, 0);
    clear_players();
    // Draw a simple vertical bar sprite for PLAYER0
    load_sprite(car_sprite_0, 0);  // Show starting sprite

    // Position player in the center
    POKE(HPOSP0, xpos);

    while (1) {
        update_scroll();
    }
}

