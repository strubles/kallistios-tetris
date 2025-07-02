# KallistiOS Tetris
A simple Tetris implementation for the Sega Dreamcast written in C.

## No game engine
This game was developed without the use of a game engine, just using KallistiOS and its low-level PowerVR rendering API. (Parallax graphics library is used for font rendering.)

## Features
* Runs on real Dreamcast hardware and emulator (e.g. nullDC)
* Standard 7-bag tetromino randomizer
* Official Tetris Guideline SRS (Super Rotation System)-compliant tetromino rotation logic
* Hard drop, soft drop, ghost piece
* Standard Tetris gravity acceleration per level
* Controller inputs

## Running
* program.cdi can be burned to a CD-R and used in most models of Dreamcast.
* Both program.cdi and program/out/program.elf can be loaded in a Dreamcast emulator.
* The .elf in program/out can be used in dc-load-ser and dc-tool to load into the Dreamcast's RAM over a serial cable (coder's cable); refer to dev_setup.md for more information.
