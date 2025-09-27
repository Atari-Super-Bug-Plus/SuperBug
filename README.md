Project Overview
Project Overview

The Superbug Capstone Project recreated Atari’s 1977 arcade game Superbug for the Atari 8-bit computer line. The game was developed in C using the Atari C libraries and compiled with the cc65 compiler into a .xex file, playable on both emulators and native Atari hardware.

The project’s goal was to faithfully reimagine Superbug while working under the strict memory, resolution, and performance constraints of the Atari 1200XL and 800 systems. It emphasized low-level programming, tile-map graphics, sprite animation, scrolling mechanisms, collision logic, sound design, and user interface development.

Key Features

```
Scrolling Tile-Map: Implemented a dynamic tile-based track with horizontal scrolling.

Sprite-Based Animation: Developed an 8-directional car sprite using player/missile graphics.

Collision Detection: Added logic for boundary crashes and scoring checkpoints.

Rotary Controller Input: Supported orientation and movement control through PORTA register reads.

HUD Display: Real-time score and fuel logic with a top-line display.

Sound Effects: Engine noise and crash sounds generated via the Atari POKEY chip.

Cross-Platform Testing: Verified functionality on both the Altirra emulator and physical Atari hardware.
```

Significant Implementations

```
Display List Management: Built a structured display list for handling mixed text/graphics modes.

Scrolling Algorithm: Created a flow for smooth directional scrolling tied to input timing.

Gas and Scoring Logic: Integrated fuel depletion and checkpoint scoring into gameplay loop.

Rotary Controller Handling: Implemented orientation updates by decoding PORTA register values.

Sound Generation: Programmed POKEY registers for frequency, distortion, and volume control.

Memory Management: Allocated and optimized data structures to avoid conflicts with OS-reserved memory.
```
