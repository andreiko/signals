#ifndef SIGNALS_ANIMATIONS_H
#define SIGNALS_ANIMATIONS_H

#include "frame.h"

Animation turningLeft = {
    .count=4,
    .frames=(Frame[4]) {
        {
            .duration=32,
            .sides=0b001000,
            .rows={
                0b00001, // o o o o o
                0b00010, //  o o o o x
                0b00010, // x o o o x
                0b00100, //  o o o o x
                0b00100, // o o o o o
                0b00100, //  o o o o x
                0b00010, // x o o o x
                0b00010, //  o o o o x
                0b00001, // o o o o o
            }
        },
        {
            .duration=32,
            .sides=0b010000,
            .rows={
                0b00010, // o o o o o
                0b00100, //  o o o o x
                0b00100, // x o o o x
                0b01000, //  o o o o x
                0b01000, // o o o o o
                0b01000, //  o o o o x
                0b00100, // x o o o x
                0b00100, //  o o o o x
                0b00010, // o o o o o
            }
        },
        {
            .duration=32,
            .sides=0b100000,
            .rows={
                0b00100, // o o o o o
                0b01000, //  o o o o x
                0b01000, // x o o o x
                0b10000, //  o o o o x
                0b10000, // o o o o o
                0b10000, //  o o o o x
                0b01000, // x o o o x
                0b01000, //  o o o o x
                0b00100, // o o o o o
            }
        },
        {
            .duration=32,
            .sides=0b000000,
            .rows={
                0b00000, // o o o o o
                0b00000, //  o o o o x
                0b00000, // x o o o x
                0b00000, //  o o o o x
                0b00000, // o o o o o
                0b00000, //  o o o o x
                0b00000, // x o o o x
                0b00000, //  o o o o x
                0b00000, // o o o o o
            }
        },
    }
};

Animation turningRight = {
    .count=4,
    .frames=(Frame[4]) {
        {
            .duration=32,
            .sides=0b000001,
            .rows={
                0b10000, // o o o o o
                0b10000, //  o o o o x
                0b01000, // x o o o x
                0b01000, //  o o o o x
                0b00100, // o o o o o
                0b01000, //  o o o o x
                0b01000, // x o o o x
                0b10000, //  o o o o x
                0b10000, // o o o o o
            }
        },
        {
            .duration=32,
            .sides=0b000010,
            .rows={
                0b01000, // o o o o o
                0b01000, //  o o o o x
                0b00100, // x o o o x
                0b00100, //  o o o o x
                0b00010, // o o o o o
                0b00100, //  o o o o x
                0b00100, // x o o o x
                0b01000, //  o o o o x
                0b01000, // o o o o o
            }
        },
        {
            .duration=32,
            .sides=0b000100,
            .rows={
                0b00100, // o o o o o
                0b00100, //  o o o o x
                0b00010, // x o o o x
                0b00010, //  o o o o x
                0b00001, // o o o o o
                0b00010, //  o o o o x
                0b00010, // x o o o x
                0b00100, //  o o o o x
                0b00100, // o o o o o
            }
        },
        {
            .duration=32,
            .sides=0b000000,
            .rows={
                0b00000, // o o o o o
                0b00000, //  o o o o x
                0b00000, // x o o o x
                0b00000, //  o o o o x
                0b00000, // o o o o o
                0b00000, //  o o o o x
                0b00000, // x o o o x
                0b00000, //  o o o o x
                0b00000, // o o o o o
            }
        },
    }
};


Animation dot = {
    .count=1,
    .frames=(Frame[1]) {
        {
            .duration=255,
            .sides=0b000000,
            .rows={
                0b00000, // o o o o o
                0b00000, //  o o o o x
                0b00100, // x o o o x
                0b01100, //  o o o o x
                0b01110, // o o o o o
                0b01100, //  o o o o x
                0b00100, // x o o o x
                0b00000, //  o o o o x
                0b00000, // o o o o o
            }
        },
    }
};

#endif //SIGNALS_ANIMATIONS_H
