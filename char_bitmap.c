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

#define CUSTOM_CHARSET ((unsigned char*)0x5000)

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 20
#define MAP_WIDTH 200
#define MAP_HEIGHT 100

unsigned char* charmap;
unsigned char* screen;

int i; // cc65: must declare all variables first
void install_display_list()
{
    unsigned char* dl = (unsigned char*)0xB800;  // Safe high RAM
    unsigned int screen_addr = *((unsigned int*)0x58);  // Current screen RAM

    dl[0] = 0x70;          // 8 blank lines
    dl[1] = 0x44;          // Mode 4 + LMS
    dl[2] = screen_addr & 0xFF;
    dl[3] = (screen_addr >> 8) & 0xFF;

    for (i = 0; i < 23; i++) {
        dl[4 + i] = 0x04;
    }

    dl[27] = 0x41;  // JMP
    dl[28] = 0x00;
    dl[29] = 0xB8;

    POKE(560, 0x00);  // low byte of 0xB800
    POKE(561, 0xB8);  // high byte of 0xB800
}
void characterMap()
{

    int row;
    int col;
    int camera_x;
    int camera_y;

    screen = (unsigned char*)(*((unsigned int*)0x58)); // Screen memory pointer

    camera_x = 0;
    camera_y = 0;

    POKE(709, 15);



    CUSTOM_CHARSET[1 * 8 + 0] = 0b11111111;
    CUSTOM_CHARSET[1 * 8 + 1] = 0b10101010;
    CUSTOM_CHARSET[1 * 8 + 2] = 0b00000000;
    CUSTOM_CHARSET[1 * 8 + 3] = 0b00000000;
    CUSTOM_CHARSET[1 * 8 + 4] = 0b00000000;
    CUSTOM_CHARSET[1 * 8 + 5] = 0b00000000;
    CUSTOM_CHARSET[1 * 8 + 6] = 0b00000000;
    CUSTOM_CHARSET[1 * 8 + 7] = 0b00000000;


    CUSTOM_CHARSET[2 * 8 + 0] = 0b00000000;
    CUSTOM_CHARSET[2 * 8 + 1] = 0b00000000;
    CUSTOM_CHARSET[2 * 8 + 2] = 0b10101010;
    CUSTOM_CHARSET[2 * 8 + 3] = 0b10101010;
    CUSTOM_CHARSET[2 * 8 + 4] = 0b00000000;
    CUSTOM_CHARSET[2 * 8 + 5] = 0b00000000;
    CUSTOM_CHARSET[2 * 8 + 6] = 0b00000000;
    CUSTOM_CHARSET[2 * 8 + 7] = 0b00000000;

    CUSTOM_CHARSET[3 * 8 + 0] = 0b00000000;
    CUSTOM_CHARSET[3 * 8 + 1] = 0b00000000;
    CUSTOM_CHARSET[3 * 8 + 2] = 0b00000000;
    CUSTOM_CHARSET[3 * 8 + 3] = 0b00000000;
    CUSTOM_CHARSET[3 * 8 + 4] = 0b10101010;
    CUSTOM_CHARSET[3 * 8 + 5] = 0b10101010;
    CUSTOM_CHARSET[3 * 8 + 6] = 0b00000000;
    CUSTOM_CHARSET[3 * 8 + 7] = 0b00000000;

    CUSTOM_CHARSET[4 * 8 + 0] = 0b00000000;
    CUSTOM_CHARSET[4 * 8 + 1] = 0b00000000;
    CUSTOM_CHARSET[4 * 8 + 2] = 0b00000000;
    CUSTOM_CHARSET[4 * 8 + 3] = 0b00000000;
    CUSTOM_CHARSET[4 * 8 + 4] = 0b00000000;
    CUSTOM_CHARSET[4 * 8 + 5] = 0b00000000;
    CUSTOM_CHARSET[4 * 8 + 6] = 0b10101010;
    CUSTOM_CHARSET[4 * 8 + 7] = 0b10101010;

    CUSTOM_CHARSET[5 * 8 + 0] = 0b00001010;
    CUSTOM_CHARSET[5 * 8 + 1] = 0b00001010;
    CUSTOM_CHARSET[5 * 8 + 2] = 0b00001010;
    CUSTOM_CHARSET[5 * 8 + 3] = 0b00001010;
    CUSTOM_CHARSET[5 * 8 + 4] = 0b10100000;
    CUSTOM_CHARSET[5 * 8 + 5] = 0b10100000;
    CUSTOM_CHARSET[5 * 8 + 6] = 0b10100000;
    CUSTOM_CHARSET[5 * 8 + 7] = 0b10100000;

    CUSTOM_CHARSET[6 * 8 + 0] = 0b10000000;
    CUSTOM_CHARSET[6 * 8 + 1] = 0b10000000;
    CUSTOM_CHARSET[6 * 8 + 2] = 0b10000000;
    CUSTOM_CHARSET[6 * 8 + 3] = 0b10000000;
    CUSTOM_CHARSET[6 * 8 + 4] = 0b00100000;
    CUSTOM_CHARSET[6 * 8 + 5] = 0b00100000;
    CUSTOM_CHARSET[6 * 8 + 6] = 0b00100000;
    CUSTOM_CHARSET[6 * 8 + 7] = 0b00100000;
    // Vertical Road
    CUSTOM_CHARSET[8 * 8 + 0] = 0b00000010;
    CUSTOM_CHARSET[8 * 8 + 1] = 0b00000010;
    CUSTOM_CHARSET[8 * 8 + 2] = 0b00000010;
    CUSTOM_CHARSET[8 * 8 + 3] = 0b00000010;
    CUSTOM_CHARSET[8 * 8 + 4] = 0b00001000;
    CUSTOM_CHARSET[8 * 8 + 5] = 0b00001000;
    CUSTOM_CHARSET[8 * 8 + 6] = 0b00001000;
    CUSTOM_CHARSET[8 * 8 + 7] = 0b00001000;

     // Vertical Road
     CUSTOM_CHARSET[9 * 8 + 0] = 0b00100000;
     CUSTOM_CHARSET[9 * 8 + 1] = 0b00100000;
     CUSTOM_CHARSET[9 * 8 + 2] = 0b00100000;
     CUSTOM_CHARSET[9 * 8 + 3] = 0b00100000;
     CUSTOM_CHARSET[9 * 8 + 4] = 0b10000000;
     CUSTOM_CHARSET[9 * 8 + 5] = 0b10000000;
     CUSTOM_CHARSET[9 * 8 + 6] = 0b10000000;
     CUSTOM_CHARSET[9 * 8 + 7] = 0b10000000;
 
    CUSTOM_CHARSET[10 * 8 + 0] = 0b10000000;
    CUSTOM_CHARSET[10 * 8 + 1] = 0b10000000;
    CUSTOM_CHARSET[10 * 8 + 2] = 0b10000000;
    CUSTOM_CHARSET[10 * 8 + 3] = 0b10000000;
    CUSTOM_CHARSET[10 * 8 + 4] = 0b10000000;
    CUSTOM_CHARSET[10 * 8 + 5] = 0b10000000;
    CUSTOM_CHARSET[10 * 8 + 6] = 0b10000000;
    CUSTOM_CHARSET[10 * 8 + 7] = 0b10000000;

    CUSTOM_CHARSET[11 * 8 + 0] = 0b00001000;
    CUSTOM_CHARSET[11 * 8 + 1] = 0b00001000;
    CUSTOM_CHARSET[11 * 8 + 2] = 0b00001000;
    CUSTOM_CHARSET[11 * 8 + 3] = 0b00001000;
    CUSTOM_CHARSET[11 * 8 + 4] = 0b00000010;
    CUSTOM_CHARSET[11 * 8 + 5] = 0b00000010;
    CUSTOM_CHARSET[11 * 8 + 6] = 0b00000010;
    CUSTOM_CHARSET[11 * 8 + 7] = 0b00000010;

    CUSTOM_CHARSET[12 * 8 + 0] = 0b00000000;
    CUSTOM_CHARSET[12 * 8 + 1] = 0b00000000;
    CUSTOM_CHARSET[12 * 8 + 2] = 0b00101000;
    CUSTOM_CHARSET[12 * 8 + 3] = 0b00101000;
    CUSTOM_CHARSET[12 * 8 + 4] = 0b00101000;
    CUSTOM_CHARSET[12 * 8 + 5] = 0b00101000;
    CUSTOM_CHARSET[12 * 8 + 6] = 0b00000000;
    CUSTOM_CHARSET[12 * 8 + 7] = 0b00000000;


    // Character sets to design the curve

    // CHAR 14: center of curve
    for (i = 0; i < 8; i += 2) {
        CUSTOM_CHARSET[0 * 8 + i] = 0;  // Clear CHAR 0
    }


    // Fill bigmap with a visible character

    for (i = 15; i < 100; i += 2)
    {
        charmap[3 * MAP_WIDTH + i] = 12;
    }
    charmap[3 * MAP_WIDTH + 14] = 12;
    charmap[3 * MAP_WIDTH + 13] = 12;
    charmap[3 * MAP_WIDTH + 12] = 12;
    charmap[4 * MAP_WIDTH + 11] = 12;
    charmap[5 * MAP_WIDTH + 10] = 12;
    charmap[6 * MAP_WIDTH + 9] = 12;
    charmap[7 * MAP_WIDTH + 9] = 12;
    //charmap[8][9] = 10;

    for (i = 25; i < 100; i++)
    {
        charmap[10 * MAP_WIDTH + i] = 1;
    }
    charmap[10 * MAP_WIDTH + 24] = 2;
    charmap[10 * MAP_WIDTH + 23] = 3;
    charmap[10 * MAP_WIDTH + 22] = 4;
    charmap[11 * MAP_WIDTH + 21] = 8;
    charmap[12 * MAP_WIDTH + 21] = 9;


    for (i = 8; i < 75; i++)
    {
        charmap[i * MAP_WIDTH + 9] = 10;
    }

    //charmap[64 * MAP_WIDTH + 9] = 6;
    //charmap[65 * MAP_WIDTH + 9] = 11;

    for (i = 13; i < 60; i++)
    {
        charmap[i * MAP_WIDTH + 21] = 10;
    }


    charmap[60 * MAP_WIDTH + 21] = 6;
    charmap[61 * MAP_WIDTH + 21] = 11;
    charmap[62 * MAP_WIDTH + 22] = 1;
    charmap[62 * MAP_WIDTH + 23] = 2;
    charmap[62 * MAP_WIDTH + 24] = 3;

    for (i = 25; i < 60; i++)
    {
        charmap[62 * MAP_WIDTH + i] = 4;
    }

    for (i = 24; i < 60; i++)
    {
        charmap[70 * MAP_WIDTH + i] = 4;
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

void main(void)
{
    _graphics(13); // Set graphics 13 mode

    // Move screen memory to known safe place


    POKE(88, 0x60); // Set screen memory to 0x6000
    charmap = (unsigned char*)0x6000;

    memset(charmap, 0, MAP_WIDTH * MAP_HEIGHT);
    memcpy(CUSTOM_CHARSET, (void*)0xE000, 1024);

    POKE(756, 80); // Tell ANTIC to use 0x5000 custom charset
    install_display_list();
    characterMap(); // Build screen

    while (1); // Keep program running
}



