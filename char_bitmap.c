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

#define CUSTOM_CHARSET ((unsigned char*)0xA000)

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 20
#define MAP_WIDTH 200
#define MAP_HEIGHT 100

unsigned char* charmap;
unsigned char* screen;

int i; // cc65: must declare all variables first

void install_display_list() {
    unsigned char* dl;
    int rows;
    unsigned int screen_addr;
    
    screen_addr = (unsigned int)screen;
    dl = (unsigned char*)0x9800;
    rows = SCREEN_HEIGHT;
    memset(dl, 0, 256);

    dl[0] = 0x70;  // 8 blank lines
    dl[1] = 0x44;  // Mode 4 + LMS
    dl[2] = screen_addr & 0xFF;
    dl[3] = (screen_addr >> 8) & 0xFF;

    for (i = 0; i < rows - 1; i++) {
        dl[4 + i] = 0x04;
    }
    dl[4 + rows] = 0x41;  // JMP
    dl[5 + rows] = ((unsigned int)dl) & 0xFF;
    dl[6 + rows] = ((unsigned int)dl) >> 8;


    dl[4 + rows] = 0x41;  // JMP
    dl[5 + rows] = ((unsigned int)dl) & 0xFF;
    dl[6 + rows] = ((unsigned int)dl) >> 8;

    POKE(560, 0x00);
    POKE(561, 0x98);
}


void characterMap()
{

    int row;
    int col;
    int camera_x;
    int camera_y;

   

    camera_x = 40;
    camera_y = 80;

    POKE(709, 15);

    CUSTOM_CHARSET[12 * 8 + 0] = 0b00000000;
    CUSTOM_CHARSET[12 * 8 + 1] = 0b00000000;
    CUSTOM_CHARSET[12 * 8 + 2] = 0b00101000;
    CUSTOM_CHARSET[12 * 8 + 3] = 0b00101000;
    CUSTOM_CHARSET[12 * 8 + 4] = 0b00101000;
    CUSTOM_CHARSET[12 * 8 + 5] = 0b00101000;
    CUSTOM_CHARSET[12 * 8 + 6] = 0b00000000;
    CUSTOM_CHARSET[12 * 8 + 7] = 0b00000000;


    //clear char 0
    for (i = 0; i < 8; i += 2) {
        CUSTOM_CHARSET[0 * 8 + i] = 0;  // Clear CHAR 0
    }

    // square 1x1
    for (i = 15; i < 100; i += 2)
    {
        charmap[3 * MAP_WIDTH + i] = 12;
    }
    charmap[4 * MAP_WIDTH + 14] = 12;
    charmap[4 * MAP_WIDTH + 13] = 12;
    charmap[4 * MAP_WIDTH + 12] = 12;

    charmap[5 * MAP_WIDTH + 11] = 12;
    charmap[5 * MAP_WIDTH + 10] = 12;

    charmap[6 * MAP_WIDTH + 9] = 12;

    charmap[7 * MAP_WIDTH + 8] = 12;
    charmap[8 * MAP_WIDTH + 8] = 12;

    charmap[9 * MAP_WIDTH + 7] = 12;
    charmap[10 * MAP_WIDTH + 7] = 12;
    charmap[11 * MAP_WIDTH + 7] = 12;

    //char 2x1 and 3x1
    for (i = 12; i < 66; i++)
    {
        charmap[i * MAP_WIDTH + 6] = 12;
    }
    
    charmap[66 * MAP_WIDTH + 7] = 12;
    charmap[67 * MAP_WIDTH + 7] = 12;
    charmap[68 * MAP_WIDTH + 7] = 12;

    charmap[69 * MAP_WIDTH + 8] = 12;
    charmap[70 * MAP_WIDTH + 8] = 12;

    charmap[71 * MAP_WIDTH + 9] = 12;

    charmap[72 * MAP_WIDTH + 10] = 12;
    charmap[72 * MAP_WIDTH + 11] = 12;
    
    charmap[73 * MAP_WIDTH + 12] = 12;
    charmap[73 * MAP_WIDTH + 13] = 12;
    charmap[73 * MAP_WIDTH + 14] = 12;

    for (i = 25; i < 100; i += 2)
    {
        charmap[13 * MAP_WIDTH + i] = 12;
    }
    charmap[14 * MAP_WIDTH + 24] = 12;
    charmap[14 * MAP_WIDTH + 23] = 12;

    charmap[15 * MAP_WIDTH + 22] = 12;

    charmap[16 * MAP_WIDTH + 21] = 12;
    charmap[17 * MAP_WIDTH + 21] = 12;


    for (i = 18; i < 60; i++)
    {
        charmap[i * MAP_WIDTH + 20] = 12;
    }


    charmap[60 * MAP_WIDTH + 21] = 12;
    charmap[61 * MAP_WIDTH + 21] = 12;

    charmap[62 * MAP_WIDTH + 22] = 12;

    charmap[63 * MAP_WIDTH + 23] = 12;
    charmap[63 * MAP_WIDTH + 24] = 12;

    for (i = 25; i < 71; i += 2)
    {
        charmap[64 * MAP_WIDTH + i] = 12;
    }

    for (i = 15; i < 61; i += 2)
    {
        charmap[74 * MAP_WIDTH + i] = 12;
    }

    // square 3x2

    charmap[65 * MAP_WIDTH + 70] = 12;
    charmap[65 * MAP_WIDTH + 71] = 12;
    charmap[65 * MAP_WIDTH + 72] = 12;

    charmap[66 * MAP_WIDTH + 73] = 12;
    charmap[66 * MAP_WIDTH + 74] = 12;

    charmap[67 * MAP_WIDTH + 75] = 12;

    charmap[68 * MAP_WIDTH + 76] = 12;
    charmap[69 * MAP_WIDTH + 76] = 12;

    charmap[70 * MAP_WIDTH + 77] = 12;
    charmap[71 * MAP_WIDTH + 77] = 12;
    charmap[72 * MAP_WIDTH + 77] = 12;

    for (i = 73; i < 85; i++)
    {
        charmap[i * MAP_WIDTH + 78] = 12;
    }

    charmap[75 * MAP_WIDTH + 60] = 12;
    charmap[75 * MAP_WIDTH + 61] = 12;

    charmap[76 * MAP_WIDTH + 62] = 12;

    charmap[77 * MAP_WIDTH + 63] = 12;
    charmap[78 * MAP_WIDTH + 63] = 12;


    for (i = 73; i < 85; i++)
    {
        charmap[i * MAP_WIDTH + 78] = 12;
    }
    
    for (i = 79; i < 85; i++)
    {
        charmap[i * MAP_WIDTH + 64] = 12;
    }

    // Copy the visible 40x24 window to the screen
    for (row = 0; row < SCREEN_HEIGHT; row++)
    {
        for (col = 0; col < SCREEN_WIDTH; col++)
        {
            screen[row * SCREEN_WIDTH + col] = charmap[(camera_y + row) * MAP_WIDTH + (camera_x + col)];
        }
    }
}
void main(void) {

    _graphics(13); // ANTIC mode 4 (text, 40 col)

    screen = (unsigned char*)0x9300;
    charmap = (unsigned char*)0x4000;
    POKE(88, 0x00);
    POKE(89, 0x93);
    printf("SCREEN @ %p\n", screen);
    memset(charmap, 0, MAP_WIDTH * MAP_HEIGHT);
    memcpy(CUSTOM_CHARSET, (void*)0xE000, 1024);  // Copy ROM charset
    POKE(756, 0xA0);  // Point ANTIC to custom charset

    install_display_list();
    characterMap();

    while (1);  // run forever
}


