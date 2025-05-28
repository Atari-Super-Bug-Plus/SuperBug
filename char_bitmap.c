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
#define SCREEN_ADDR   ((unsigned char*)0xB200)
#define DISPLAY_LIST  0xBA00
#define HSCROL  (*(unsigned char*)0xD404)  // Horizontal fine scroll register
#define VCOUNT  (*(volatile unsigned char*)0xD40B)  // Current scanline
#define SCREEN_STRIDE 41
#define SCREEN_HEIGHT 24
#define SCREEN_WIDTH  40
#define MAP_WIDTH     200
#define MAP_HEIGHT    100

unsigned char* charmap;
unsigned char* screen;
unsigned char scroll_offset = 0;
unsigned char pending_scroll_reset = 0;
int camera_x = 0;
int camera_y = 0;
int row, col, i, j;
unsigned char scroll_speed = 1;
unsigned char scroll_hold_counter = 0;



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
        line_addr = SCREEN_ADDR + i * SCREEN_STRIDE;
        dl[j++] = 0x54 | 0x10;  // Mode 4 + LMS + fine scroll
        dl[j++] = line_addr & 0xFF;
        dl[j++] = (line_addr >> 8) & 0xFF;
    }

    dl[j++] = 0x41;  // JVB
    dl[j++] = ((unsigned int)dl) & 0xFF;
    dl[j++] = ((unsigned int)dl) >> 8;
}

void draw_map() {
    unsigned char* dst = screen;
    unsigned char* src = &charmap[camera_y * MAP_WIDTH + camera_x];

    for (row = 0; row < SCREEN_HEIGHT; row++) {
        memcpy(dst, src, SCREEN_WIDTH + 1);  // copy 41 columns per row
        dst += SCREEN_STRIDE;
        src += MAP_WIDTH;
    }
}
unsigned char tiles_scrolled;
void update_scroll() {
    unsigned char joy_state = joy_read(JOY_1);
    int map_row, map_col;

    while (VCOUNT > 8);
    while (VCOUNT <= 8);

    if (pending_scroll_reset) {
        HSCROL = 0;
        pending_scroll_reset = 0;
    }

    // RIGHT
    if (JOY_RIGHT(joy_state) && (camera_x + SCREEN_WIDTH + 1 < MAP_WIDTH)) {
        scroll_hold_counter++;
        if (scroll_hold_counter == 8) scroll_speed = 2;
        else if (scroll_hold_counter == 16) scroll_speed = 4;
        else if (scroll_hold_counter == 24) scroll_speed = 6;

        scroll_offset += scroll_speed;

        if (scroll_offset >= 7 && scroll_offset < 8 + scroll_speed) {
            for (row = 0; row < SCREEN_HEIGHT; row++) {
                map_row = camera_y + row;
                map_col = camera_x + SCREEN_WIDTH;
                screen[row * SCREEN_STRIDE + 40] =
                    (map_row >= MAP_HEIGHT || map_col >= MAP_WIDTH) ? 0 :
                    charmap[map_row * MAP_WIDTH + map_col];
            }
        }

        if (scroll_offset >= 8) {
            tiles_scrolled = scroll_offset / 8;
            scroll_offset %= 8;
            camera_x += tiles_scrolled;
            draw_map();
            pending_scroll_reset = 1;
        }
    }
    // LEFT
    else if (JOY_LEFT(joy_state) && camera_x > 0) {
        scroll_hold_counter++;
        if (scroll_hold_counter == 8) scroll_speed = 2;
        else if (scroll_hold_counter == 16) scroll_speed = 4;
        else if (scroll_hold_counter == 24) scroll_speed = 6;

        scroll_offset += scroll_speed;

        if (scroll_offset >= 7 && scroll_offset < 8 + scroll_speed) {
            for (row = 0; row < SCREEN_HEIGHT; row++) {
                map_row = camera_y + row;
                map_col = camera_x - 1;
                screen[row * SCREEN_STRIDE] =
                    (map_row >= MAP_HEIGHT || map_col >= MAP_WIDTH) ? 0 :
                    charmap[map_row * MAP_WIDTH + map_col];
            }
        }

        if (scroll_offset >= 8) {
            tiles_scrolled = scroll_offset / 8;
            scroll_offset %= 8;
            camera_x -= tiles_scrolled;
            draw_map();
            pending_scroll_reset = 1;
        }
    }
    // UP
    else if (JOY_UP(joy_state) && camera_y > 0) {
        scroll_hold_counter++;
        if (scroll_hold_counter == 8) scroll_speed = 1;
        else if (scroll_hold_counter == 16) scroll_speed = 2;
        else if (scroll_hold_counter == 24) scroll_speed = 3;

        camera_y -= scroll_speed;
        if (camera_y < 0) camera_y = 0;
        draw_map();
        pending_scroll_reset = 1;
    }
    // DOWN
    else if (JOY_DOWN(joy_state) && (camera_y + SCREEN_HEIGHT + 1 < MAP_HEIGHT)) {
        scroll_hold_counter++;
        if (scroll_hold_counter == 8) scroll_speed = 1;
        else if (scroll_hold_counter == 16) scroll_speed = 2;
        else if (scroll_hold_counter == 24) scroll_speed = 3;

        camera_y += scroll_speed;
        draw_map();
        pending_scroll_reset = 1;
    }
    else {
        scroll_hold_counter = 0;
        scroll_speed = 1;
    }
}

void main(void) {
    _graphics(12);  // ANTIC mode 4
    charmap = (unsigned char*)CHARMAP_ADDR;
    screen = (unsigned char*)SCREEN_ADDR;

    memset(charmap, 0, MAP_WIDTH * MAP_HEIGHT);
    memset(screen, 0, SCREEN_STRIDE * SCREEN_HEIGHT);

    memcpy(CUSTOM_CHARSET, (void*)0xE000, 1024);  // Use system ROM charset
    POKE(756, ((unsigned int)CUSTOM_CHARSET) >> 8);

    install_display_list();
    characterMap();
    draw_map();

    POKE(560, DISPLAY_LIST & 0xFF);
    POKE(561, DISPLAY_LIST >> 8);
    POKE(559, 0x22);  // DMA enable + fine scroll
    HSCROL = 0;

    joy_install(&joy_static_stddrv);

    while (1) {
        update_scroll();
    }
}


