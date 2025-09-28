### Project Overview

The Superbug Capstone Project recreated Atari’s 1977 arcade game Superbug for the Atari 8-bit computer line. The game was developed in C using the Atari C libraries and compiled with the cc65 compiler into a .xex file, playable on both emulators and native Atari hardware.

The project’s goal was to faithfully reimagine Superbug while working under the strict memory, resolution, and performance constraints of the Atari 1200XL and 800 systems. It emphasized low-level programming and memory managament.

#### Key Features

```
Display List Management: Used the built in display list to display a tilemap that was stored in memory.

Tile-Map Scrolling: Implemented a dynamic horizontal and vertical scrolling for the tilemap.

Sprite-Based Animation: Developed an 8-directional car sprite using player/missile graphics.

Collision Detection: Added logic for boundary crashes and scoring checkpoints.

Rotary Controller Input: Supported player movement controls through PORTA register reads.

HUD Display: Real-time score and fuel logic with a top-line display.

Sound Effects: Engine noise generated via the Atari POKEY chip.

Cross-Platform Testing: Verified functionality on both the Altirra emulator and physical Atari hardware.
```

#### Repository Organization

```
Atari Header Files/
Contains Atari-specific header files required for compilation. These define registers, memory locations, and constants for graphics, sound, and input.

Sprite Animation/
Holds source code for sprite rendering and animation routines. Includes car rotation logic and tests for smooth player/missile graphics handling.

Working Demos/
Two experimental builds used during development. 
The first demo named 'workingDemoJoystick.c' contains the source code that was used to test the horizontal and vertical scrolling of the tilemap
using joystick inputs. For this build, the player movement and collisions were rudimentary and only what was required for seeing if the scrolling
functioned correctly. The second demo named 'workingDemo.c' we fully implemented player graphics, collision logic, scoring logic, gas timer logic,
player controls and tilemap scrolling using the rotary controller inputs.

SUPERBUG.xex
Final compiled build of the game. Runs on the Altirra emulator and native Atari 8-bit hardware.
```


#### Team Contributions

Noah Fagerlie
```
- Worked to implement horitzon and vertical scrolling of the tilemap stored in memory.

- Utilized the built in display list to display the tile map to the screen.

- Integrated sprites into player/missile graphics and linked them to rotary controller input.

- Linked it to rotary controls to tilemap scrolling, determining scroll direction from PORTA register values.
```

Hunter Patchett
```
- Worked to implement horitzon and vertical scrolling of the tilemap stored in memory.

- Implemented scoring logic, collision detection, and gas timer systems.

- Built the HUD to display score, fuel, and other values.

- Managed tile map storage in memory and used the display list for rendering to the screen.

- Modified the top display line to a different graphics mode for HUD information.
```

Andrew Link
```
- Helped designed and created car sprites.

- Positioned sprites correctly in memory.

- Assisted with sprite rotation and animation logic.
```

Jason Truong
```
- Contributed to project documentation.

- Conducted testing and validation throughout development.
```

