# Project 2 — Traffic Light Robot
**Author:** Samah Ahmed Mahmoud Ahmed  
**Email:** [sammahmedzz50@gmail.com](mailto:sammahmedzz50@gmail.com)
## Description
A traffic light controller simulation supporting state transitions, tick-based timing, dynamic car queues, night mode blinking, and comprehensive crossing logs.

## How to Build and Run
1. Open terminal in the project directory.
2. Compile with warnings enabled:
   ```bash
   gcc -Wall -Wextra main.c -o traffic_light
Run the executable:

On Windows: .\traffic_light.exe

On Linux/Mac: ./traffic_light

Explain Why
How does Night Mode operate?
It overrides normal countdown sequences and uses bitwise operations to toggle the yellow flashing light state without mutating unrelated system parameters or resetting tick counters.
