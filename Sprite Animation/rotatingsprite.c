#include <atari.h>
#include <peekpoke.h>
#include <string.h>  // For memset/memcpy functions

// Player/Missile Graphics definitions
#define PMG_MEM 0x3000
#define PLAYER0_GFX (PMG_MEM + 0x200)
#define PLAYER1_GFX (PMG_MEM + 0x280)

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

// Car sprite definition for up direction (8 pixels wide)
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

unsigned char xpos = 120;
unsigned char direction = DIR_UP;
unsigned char using_two_players = 0;  // Flag to track if we're using both players

void clear_players(void) {
    // Use memset for efficiency
    memset((void*)(PLAYER0_GFX), 0, 256);
    memset((void*)(PLAYER1_GFX), 0, 256);
}

void load_sprite(unsigned char* sprite, unsigned char player) {
    unsigned int i;
    unsigned int base = (player == 0) ? PLAYER0_GFX : PLAYER1_GFX;
    for (i = 0; i < 16; i++) {
        POKE(base + 50 + i, sprite[i]);
    }
    if (player == 0) {
        for (i = 0; i < 16; i++) {
            POKE(PLAYER1_GFX + 50 + i, 0);  // Clear player 1 when using just player 0
        }
    }
}

void load_right_facing_sprite(void) {
    unsigned int i;
    // Only clear the sprite area (not entire player memory)
    for (i = 0; i < 16; i++) {
        POKE(PLAYER0_GFX + 50 + i, car_sprite_right_p0[i]); // Left half
        POKE(PLAYER1_GFX + 50 + i, car_sprite_right_p1[i]); // Right half
    }
    // Position player 1 exactly 8 pixels to the right of player 0
    POKE(HPOSP1, xpos + 8);
}

void load_left_facing_sprite(void) {
    unsigned int i;
    // Only clear the sprite area (not entire player memory)
    for (i = 0; i < 16; i++) {
        POKE(PLAYER0_GFX + 50 + i, car_sprite_left_p0[i]); // Left half
        POKE(PLAYER1_GFX + 50 + i, car_sprite_left_p1[i]); // Right half
    }
    // Position player 1 exactly 8 pixels to the right of player 0
    POKE(HPOSP1, xpos + 8);
}

void main(void) {
    unsigned char stick;
    unsigned char prev_direction = 255;  // Invalid value to force initial sprite update

    // Initialize position and direction
    xpos = 120;
    direction = DIR_UP;
    
    // Initialize the Player/Missile Graphics system
    POKE(710, 0);               // Background black
    POKE(DMACTL, 0x2E);         // Enable PMG + narrow playfield
    POKE(GRACTL, 0x03);         // Enable player graphics
    POKE(PMBASE, PMG_MEM >> 8); // Set PMG base
    
    // Set player colors and sizes
    POKE(COLOR0, 254);          // Yellow for P0
    POKE(COLOR1, 254);          // Yellow for P1 (same color)
    POKE(SIZEP0, 0);            // Normal width for player 0
    POKE(SIZEP1, 0);            // Normal width for player 1
    
    // Initialize player positions
    POKE(HPOSP0, xpos);
    
    // Clear player memory and load initial sprite
    clear_players();
    load_sprite(car_sprite_0, 0);
    
    // Main game loop
    while (1) {
        // Read joystick
        stick = PEEK(STICK0);

        // Update direction based on joystick
        switch (stick) {
            case 14: direction = DIR_UP; break;     // Up
            case 13: direction = DIR_DOWN; break;   // Down
            case 11: direction = DIR_LEFT; break;   // Left
            case 7:  direction = DIR_RIGHT; break;  // Right
        }

        // Only update graphics if direction changes
        if (direction != prev_direction) {
            // Handle sprite loading based on direction
            if (direction == DIR_RIGHT) {
                load_right_facing_sprite();
                using_two_players = 1;  // We're using both players
            } else if (direction == DIR_LEFT) {
                load_left_facing_sprite();
                using_two_players = 1;  // We're using both players
            } else {
                if (direction == DIR_UP) {
                    load_sprite(car_sprite_0, 0);
                } else if (direction == DIR_DOWN) {
                    load_sprite(car_sprite_180, 0);
                }
                using_two_players = 0;  // Only using player 0
            }
            
            prev_direction = direction;
        }

        // Always update player 0 position
        POKE(HPOSP0, xpos);
        
        // Update player 1 position if we're using two players
        if (using_two_players) {
            POKE(HPOSP1, xpos + 8);
        }
    }
}
