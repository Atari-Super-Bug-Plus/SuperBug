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
#define MAP_WIDTH 200U
#define MAP_HEIGHT 100U

unsigned char charmap[MAP_WIDTH * MAP_HEIGHT];
unsigned char* screen;
unsigned char joy;

int i; // cc65: must declare all variables first

int row;
int col;
int camera_x = 40;
int camera_y = 80;

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

    for (i = 0; i < rows; i++) {
        dl[4 + i] = 0x04;
    }
    dl[4 + rows] = 0x41;  // JMP
    dl[5 + rows] = ((unsigned int)dl) & 0xFF;
    dl[6 + rows] = ((unsigned int)dl) >> 8;

    POKE(560, 0x00);
    POKE(561, 0x98);
}


void characterMap()
{
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
    for (i = 14; i < 102; i += 2)
    {
        charmap[2 * MAP_WIDTH + i] = 12;
    }
    charmap[3 * MAP_WIDTH + 13] = 12;
    charmap[3 * MAP_WIDTH + 12] = 12;
    charmap[3 * MAP_WIDTH + 11] = 12;

    charmap[4 * MAP_WIDTH + 10] = 12;
    charmap[4 * MAP_WIDTH + 9] = 12;

    charmap[5 * MAP_WIDTH + 8] = 12;

    charmap[6 * MAP_WIDTH + 7] = 12;
    charmap[7 * MAP_WIDTH + 7] = 12;

    charmap[8 * MAP_WIDTH + 6] = 12;
    charmap[9 * MAP_WIDTH + 6] = 12;
    charmap[10 * MAP_WIDTH + 6] = 12;

    //char 2x1 and 3x1
    for (i = 11; i < 50; i++)
    {
        charmap[i * MAP_WIDTH + 5] = 12;
    }
    
    charmap[50 * MAP_WIDTH + 6] = 12;
    charmap[51 * MAP_WIDTH + 6] = 12;
    charmap[52 * MAP_WIDTH + 6] = 12;

    charmap[53 * MAP_WIDTH + 7] = 12;
    charmap[54 * MAP_WIDTH + 7] = 12;

    charmap[55 * MAP_WIDTH + 8] = 12;

    charmap[56 * MAP_WIDTH + 9] = 12;
    charmap[56 * MAP_WIDTH + 10] = 12;
    
    charmap[57 * MAP_WIDTH + 11] = 12;
    charmap[57 * MAP_WIDTH + 12] = 12;
    charmap[57 * MAP_WIDTH + 13] = 12;

    for (i = 30; i < 86; i += 2)
    {
        charmap[15 * MAP_WIDTH + i] = 12;
    }
    charmap[16 * MAP_WIDTH + 29] = 12;
    charmap[16 * MAP_WIDTH + 28] = 12;

    charmap[17 * MAP_WIDTH + 27] = 12;

    charmap[18 * MAP_WIDTH + 26] = 12;
    charmap[19 * MAP_WIDTH + 26] = 12;


    for (i = 20; i < 42; i++)
    {
        charmap[i * MAP_WIDTH + 25] = 12;
    }


    charmap[42 * MAP_WIDTH + 26] = 12;
    charmap[43 * MAP_WIDTH + 26] = 12;

    charmap[44 * MAP_WIDTH + 27] = 12;

    charmap[45 * MAP_WIDTH + 28] = 12;
    charmap[45 * MAP_WIDTH + 29] = 12;

    for (i = 30; i < 71; i += 2)
    {
        charmap[46 * MAP_WIDTH + i] = 12;
    }

    for (i = 14; i < 55; i += 2)
    {
        charmap[58 * MAP_WIDTH + i] = 12;
    }

    // square 3x2

    charmap[47 * MAP_WIDTH + 71] = 12;
    charmap[47 * MAP_WIDTH + 72] = 12;
    charmap[47 * MAP_WIDTH + 73] = 12;

    charmap[48 * MAP_WIDTH + 74] = 12;
    charmap[48 * MAP_WIDTH + 75] = 12;

    charmap[49 * MAP_WIDTH + 76] = 12;

    charmap[50 * MAP_WIDTH + 77] = 12;
    charmap[51 * MAP_WIDTH + 77] = 12;

    charmap[52 * MAP_WIDTH + 78] = 12;
    charmap[53 * MAP_WIDTH + 78] = 12;
    charmap[54 * MAP_WIDTH + 78] = 12;

    for (i = 55; i < 70; i++)
    {
        charmap[i * MAP_WIDTH + 79] = 12;
    }

    charmap[59 * MAP_WIDTH + 55] = 12;
    charmap[59 * MAP_WIDTH + 56] = 12;

    charmap[60 * MAP_WIDTH + 57] = 12;

    charmap[61 * MAP_WIDTH + 58] = 12;
    charmap[62 * MAP_WIDTH + 58] = 12;

    charmap[63* MAP_WIDTH + 57] = 12;
    charmap[64* MAP_WIDTH + 57] = 12;

    charmap[65* MAP_WIDTH + 56] = 12;

    charmap[66* MAP_WIDTH + 55] = 12;
    charmap[66* MAP_WIDTH + 54] = 12;

    for (i = 53; i > 8; i -= 2)
    {
        charmap[67 * MAP_WIDTH + i] = 12;
    }

    charmap[70* MAP_WIDTH + 78] = 12;
    charmap[71* MAP_WIDTH + 78] = 12;
    charmap[72* MAP_WIDTH + 78] = 12;

    charmap[73* MAP_WIDTH + 77] = 12;
    charmap[74* MAP_WIDTH + 77] = 12;

    charmap[75* MAP_WIDTH + 76] = 12;

    charmap[76* MAP_WIDTH + 75] = 12;
    charmap[76* MAP_WIDTH + 74] = 12;

    charmap[77* MAP_WIDTH + 73] = 12;
    charmap[77* MAP_WIDTH + 72] = 12;
    charmap[77* MAP_WIDTH + 71] = 12;

    for (i = 69; i > 25; i -= 2)
    {
        charmap[78 * MAP_WIDTH + i] = 12;
    }


    charmap[68* MAP_WIDTH + 8] = 12;
    charmap[68* MAP_WIDTH + 7] = 12;
    charmap[68* MAP_WIDTH + 6] = 12;

    charmap[69* MAP_WIDTH + 5] = 12;
    charmap[69* MAP_WIDTH + 4] = 12;

    charmap[70* MAP_WIDTH + 3] = 12;

    charmap[71* MAP_WIDTH + 2] = 12;
    charmap[72* MAP_WIDTH + 2] = 12;

    charmap[73* MAP_WIDTH + 1] = 12;
    charmap[74* MAP_WIDTH + 1] = 12;
    charmap[75* MAP_WIDTH + 1] = 12;

    for(i = 76; i < 91; i++)
    {
        charmap[i * MAP_WIDTH + 0] = 12;
    }

    charmap[79* MAP_WIDTH + 26] = 12;
    charmap[79* MAP_WIDTH + 25] = 12;
    
    charmap[80* MAP_WIDTH + 24] = 12;

    charmap[81* MAP_WIDTH + 23] = 12;
    charmap[82* MAP_WIDTH + 23] = 12;
    charmap[83* MAP_WIDTH + 23] = 12;

    charmap[84* MAP_WIDTH + 24] = 12;

    charmap[85* MAP_WIDTH + 25] = 12;
    charmap[85* MAP_WIDTH + 26] = 12;

    for (i = 27; i < 90; i +=2)
    {
        charmap[86* MAP_WIDTH + i] = 12;
    } 

    charmap[91* MAP_WIDTH + 1] = 12;
    charmap[92* MAP_WIDTH + 1] = 12;
    charmap[93* MAP_WIDTH + 1] = 12;

    charmap[94* MAP_WIDTH + 2] = 12;
    charmap[95* MAP_WIDTH + 2] = 12;
    charmap[96* MAP_WIDTH + 3] = 12;

    charmap[97* MAP_WIDTH + 4] = 12;
    charmap[97* MAP_WIDTH + 5] = 12;
    
    charmap[98* MAP_WIDTH + 6] = 12;
    charmap[98* MAP_WIDTH + 7] = 12;
    charmap[98* MAP_WIDTH + 8] = 12;

    for (i = 9; i < 104; i+= 2)
    {
        charmap[99* MAP_WIDTH + i] = 12;
    }


    charmap[98* MAP_WIDTH + 104] = 12;
    charmap[98* MAP_WIDTH + 105] = 12;
    charmap[98* MAP_WIDTH + 106] = 12;

    charmap[97* MAP_WIDTH + 107] = 12;
    charmap[97* MAP_WIDTH + 108] = 12;

    charmap[96* MAP_WIDTH + 109] = 12;

    charmap[95* MAP_WIDTH + 110] = 12;
    charmap[94* MAP_WIDTH + 110] = 12;

    charmap[93* MAP_WIDTH + 111] = 12;
    charmap[92* MAP_WIDTH + 111] = 12;
    charmap[91* MAP_WIDTH + 111] = 12;


    charmap[85* MAP_WIDTH + 90] = 12;
    charmap[85* MAP_WIDTH + 91] = 12;
    charmap[85* MAP_WIDTH + 92] = 12;

    charmap[84* MAP_WIDTH + 93] = 12;
    charmap[84* MAP_WIDTH + 94] = 12;

    charmap[83* MAP_WIDTH + 95] = 12;

    charmap[82* MAP_WIDTH + 96] = 12;
    charmap[81* MAP_WIDTH + 96] = 12;

    charmap[80* MAP_WIDTH + 97] = 12;
    charmap[79* MAP_WIDTH + 97] = 12;
    charmap[78* MAP_WIDTH + 97] = 12;


    for (i = 90; i > 55; i--)
    {
        charmap[i * MAP_WIDTH + 112] = 12;
    }

    for (i = 77; i > 50; i--)
    {
        charmap[i* MAP_WIDTH + 98] = 12;
    }

    charmap[55 * MAP_WIDTH + 113] = 12;
    charmap[54 * MAP_WIDTH + 113] = 12;

    charmap[53 * MAP_WIDTH + 114] = 12;

    charmap[52 * MAP_WIDTH + 115] = 12;
    charmap[52 * MAP_WIDTH + 116] = 12;
    charmap[52 * MAP_WIDTH + 117] = 12;

    charmap[53 * MAP_WIDTH + 118] = 12;

    charmap[54 * MAP_WIDTH + 119] = 12;
    charmap[55 * MAP_WIDTH + 119] = 12;

    for (i = 56; i < 91; i++)
    {
        charmap[i * MAP_WIDTH + 120] = 12;
    }

    charmap[50 * MAP_WIDTH + 99] = 12;
    charmap[49 * MAP_WIDTH + 99] = 12;
    charmap[48 * MAP_WIDTH + 99] = 12;

    charmap[47 * MAP_WIDTH + 100] = 12;
    charmap[46 * MAP_WIDTH + 100] = 12;

    charmap[45 * MAP_WIDTH + 101] = 12;

    charmap[44 * MAP_WIDTH + 102] = 12;
    charmap[44 * MAP_WIDTH + 103] = 12;

    charmap[43 * MAP_WIDTH + 104] = 12;
    charmap[43 * MAP_WIDTH + 105] = 12;
    charmap[43 * MAP_WIDTH + 106] = 12;


    for (i = 107; i < 130; i += 2)
    {
        charmap[42 * MAP_WIDTH + i] = 12;
    }

    charmap[43 * MAP_WIDTH + 130] = 12;
    charmap[43 * MAP_WIDTH + 131] = 12;
    charmap[43 * MAP_WIDTH + 132] = 12;

    charmap[44 * MAP_WIDTH + 133] = 12;
    charmap[44 * MAP_WIDTH + 134] = 12;

    charmap[45 * MAP_WIDTH + 135] = 12;

    charmap[46 * MAP_WIDTH + 136] = 12;
    charmap[47 * MAP_WIDTH + 136] = 12;
    
    charmap[48 * MAP_WIDTH + 137] = 12;
    charmap[49 * MAP_WIDTH + 137] = 12;
    charmap[50 * MAP_WIDTH + 137] = 12;

    for (i = 51; i < 81; i++)
    {
        charmap[i * MAP_WIDTH + 138] = 12;
    }

    charmap[91 * MAP_WIDTH + 121] = 12;
    charmap[92 * MAP_WIDTH + 121] = 12;
    charmap[93 * MAP_WIDTH + 121] = 12;

    charmap[94 * MAP_WIDTH + 122] = 12;
    charmap[95 * MAP_WIDTH + 122] = 12;

    charmap[96 * MAP_WIDTH + 123] = 12;

    charmap[97 * MAP_WIDTH + 124] = 12;
    charmap[97 * MAP_WIDTH + 125] = 12;

    charmap[98 * MAP_WIDTH + 126] = 12;
    charmap[98 * MAP_WIDTH + 127] = 12;
    charmap[98 * MAP_WIDTH + 128] = 12;
    
    charmap[81 * MAP_WIDTH + 139] = 12;
    charmap[82 * MAP_WIDTH + 139] = 12;

    charmap[83 * MAP_WIDTH + 140] = 12;

    charmap[84 * MAP_WIDTH + 141] = 12;
    charmap[84 * MAP_WIDTH + 142] = 12;

    for (i = 129; i < 186; i+=2)
    {
        charmap[99 * MAP_WIDTH + i] = 12;
    }

    for (i = 143; i < 170; i+=2)
    {
        charmap[85 * MAP_WIDTH + i] = 12;
    }
    
    charmap[98 * MAP_WIDTH + 186] = 12;
    charmap[98 * MAP_WIDTH + 187] = 12;
    charmap[98 * MAP_WIDTH + 188] = 12;

    charmap[97 * MAP_WIDTH + 189] = 12;
    charmap[97 * MAP_WIDTH + 190] = 12;

    charmap[96 * MAP_WIDTH + 191] = 12;

    charmap[95 * MAP_WIDTH + 192] = 12;
    charmap[94 * MAP_WIDTH + 192] = 12;
    
    charmap[93 * MAP_WIDTH + 193] = 12;
    charmap[92 * MAP_WIDTH + 193] = 12;
    charmap[91 * MAP_WIDTH + 193] = 12;

    charmap[84 * MAP_WIDTH + 170] = 12;
    charmap[84 * MAP_WIDTH + 171] = 12;

    charmap[83 * MAP_WIDTH + 172] = 12;

    charmap[82 * MAP_WIDTH + 173] = 12;
    charmap[81 * MAP_WIDTH + 173] = 12;

    for (i = 80; i > 19; i--)
    {
        charmap[i * MAP_WIDTH + 174] = 12;
    }

    for (i = 90; i > 10; i--)
    {
        charmap[i * MAP_WIDTH + 194] = 12;
    }

    charmap[10 * MAP_WIDTH + 193] = 12;
    charmap[9 * MAP_WIDTH + 193] = 12;
    charmap[8 * MAP_WIDTH + 193] = 12;

    charmap[7 * MAP_WIDTH + 192] = 12;
    charmap[6 * MAP_WIDTH + 192] = 12;

    charmap[5 * MAP_WIDTH + 191] = 12;

    charmap[4 * MAP_WIDTH + 190] = 12;
    charmap[4 * MAP_WIDTH + 189] = 12;

    charmap[3 * MAP_WIDTH + 188] = 12;
    charmap[3 * MAP_WIDTH + 187] = 12;
    charmap[3 * MAP_WIDTH + 186] = 12;

    for (i = 185; i > 138; i -= 2)
    {
        charmap[2 * MAP_WIDTH + i] = 12;
    }

    charmap[19 * MAP_WIDTH + 173] = 12;
    charmap[18 * MAP_WIDTH + 173] = 12;

    charmap[17 * MAP_WIDTH + 172] = 12;

    charmap[16 * MAP_WIDTH + 171] = 12;
    charmap[16 * MAP_WIDTH + 170] = 12;

    for (i = 169; i > 155; i -= 2)
    {
        charmap[15 * MAP_WIDTH + i] = 12;
    }
    
    charmap[16 * MAP_WIDTH + 156] = 12;
    charmap[16 * MAP_WIDTH + 155] = 12;

    charmap[17 * MAP_WIDTH + 154] = 12;

    charmap[18 * MAP_WIDTH + 153] = 12;
    charmap[19 * MAP_WIDTH + 153] = 12;

    charmap[3 * MAP_WIDTH + 137] = 12;
    charmap[3 * MAP_WIDTH + 136] = 12;
    charmap[3 * MAP_WIDTH + 135] = 12;

    charmap[4 * MAP_WIDTH + 134] = 12;
    charmap[4 * MAP_WIDTH + 133] = 12;

    charmap[5 * MAP_WIDTH + 132] = 12;

    charmap[6 * MAP_WIDTH + 131] = 12;
    charmap[7 * MAP_WIDTH + 131] = 12;

    charmap[8 * MAP_WIDTH + 130] = 12;
    charmap[9 * MAP_WIDTH + 130] = 12;
    charmap[10 * MAP_WIDTH + 130] = 12;

    for (i = 11; i < 22; i++)
    {
        charmap[i * MAP_WIDTH + 129] = 12;
    }

    charmap[22 * MAP_WIDTH + 128] = 12;
    charmap[23 * MAP_WIDTH + 128] = 12;

    charmap[24 * MAP_WIDTH + 127] = 12;

    charmap[25 * MAP_WIDTH + 126] = 12;
    charmap[25 * MAP_WIDTH + 125] = 12;

    for (i = 20; i < 31; i++)
    {
        charmap[i * MAP_WIDTH + 152] = 12;
    }
    
    charmap[31 * MAP_WIDTH + 151] = 12;
    charmap[32 * MAP_WIDTH + 151] = 12;
    charmap[33 * MAP_WIDTH + 151] = 12;

    charmap[34 * MAP_WIDTH + 150] = 12;
    charmap[35 * MAP_WIDTH + 150] = 12;

    charmap[36 * MAP_WIDTH + 149] = 12;

    charmap[37 * MAP_WIDTH + 148] = 12;
    charmap[37 * MAP_WIDTH + 147] = 12;

    charmap[38 * MAP_WIDTH + 146] = 12;
    charmap[38 * MAP_WIDTH + 145] = 12;
    charmap[38 * MAP_WIDTH + 144] = 12;

    for (i = 143; i > 98; i-= 2)
    {
        charmap[39 * MAP_WIDTH + i] = 12;
    }
    charmap[38 * MAP_WIDTH + 98] = 12;
    charmap[38 * MAP_WIDTH + 97] = 12;
    charmap[38 * MAP_WIDTH + 96] = 12;
    
    charmap[37 * MAP_WIDTH + 95] = 12;
    charmap[37 * MAP_WIDTH + 94] = 12;
    
    charmap[36 * MAP_WIDTH + 93] = 12;

    charmap[35 * MAP_WIDTH + 92] = 12;
    charmap[34 * MAP_WIDTH + 92] = 12;
    
    charmap[33 * MAP_WIDTH + 91] = 12;
    charmap[32 * MAP_WIDTH + 91] = 12;
    charmap[31 * MAP_WIDTH + 91] = 12;

    for (i = 124; i > 115; i-= 2)
    {
        charmap[26 * MAP_WIDTH + i] = 12;
    }

    charmap[25 * MAP_WIDTH + 115] = 12;
    charmap[25 * MAP_WIDTH + 114] = 12;

    charmap[24 * MAP_WIDTH + 113] = 12;
    
    charmap[23 * MAP_WIDTH + 112] = 12;
    charmap[22 * MAP_WIDTH + 112] = 12;

    charmap[21 * MAP_WIDTH + 90] = 12;
    charmap[20 * MAP_WIDTH + 90] = 12;

    for (i = 30; i > 10; i--)
    {
        charmap[i * MAP_WIDTH + 110] = 12;
    }

    charmap[19 * MAP_WIDTH + 89] = 12;
    charmap[18 * MAP_WIDTH + 89] = 12;
  
    
    charmap[17 * MAP_WIDTH + 88] = 12;

    charmap[16 * MAP_WIDTH + 87] = 12;
    charmap[16 * MAP_WIDTH + 86] = 12;
    

    charmap[10 * MAP_WIDTH + 109] = 12;
    charmap[9 * MAP_WIDTH + 109] = 12;
    charmap[8 * MAP_WIDTH + 109] = 12;
    
    charmap[7 * MAP_WIDTH + 108] = 12;
    charmap[6 * MAP_WIDTH + 108] = 12;
    
    charmap[5 * MAP_WIDTH + 107] = 12;

    charmap[4 * MAP_WIDTH + 106] = 12;
    charmap[4 * MAP_WIDTH + 105] = 12;
    
    charmap[3 * MAP_WIDTH + 104] = 12;
    charmap[3 * MAP_WIDTH + 103] = 12;
    charmap[3 * MAP_WIDTH + 102] = 12;
}

void draw_map()
{
    // Copy the visible 40x20 window to the screen
    for (row = 0; row < SCREEN_HEIGHT; row++)
    {
        for (col = 0; col < SCREEN_WIDTH; col++)
        {
            screen[row * SCREEN_WIDTH + col] = charmap[(camera_y + row) * MAP_WIDTH + (camera_x + col)];
        }
    }
}

void update_camera() {
    unsigned char joy_state = joy_read(JOY_1);
    
    
    if (JOY_UP(joy_state) && camera_y > 0) {
        camera_y -= 5;
    }
    else if (JOY_DOWN(joy_state) && camera_y < MAP_HEIGHT - SCREEN_HEIGHT) {
        camera_y += 5;
    }
    else if (JOY_LEFT(joy_state) && camera_x > 0) {
        camera_x -= 5;
    }
    else if (JOY_RIGHT(joy_state) && camera_x < MAP_WIDTH - SCREEN_WIDTH) {
        camera_x += 5;
    }
}


void main(void) {  
    _graphics(13); // ANTIC mode 4 (text, 40 col)
    
    screen = (unsigned char*)0x9300;
    POKE(88, 0x00);
    POKE(89, 0x93);
    printf("SCREEN @ %p\n", screen);
    memset(charmap, 0, MAP_WIDTH * MAP_HEIGHT);
    memcpy(CUSTOM_CHARSET, (void*)0xE000, 1024);  // Copy ROM charset
    POKE(756, 0xA0);  // Point ANTIC to custom charset

    joy_install(&joy_static_stddrv);
    install_display_list();
    characterMap();
    
    while (1){
        update_camera();
        draw_map();
    };
}



