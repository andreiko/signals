#ifndef SIGNALS_ANIMATIONS_H
#define SIGNALS_ANIMATIONS_H

#include "frame.h"

Animation turningLeft = {
    .count=4,
    .frames=(Frame[4]) {
        {
            .duration=32,
            .sides_left=0b100,
            .sides_right=0b000,
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
            .sides_left=0b010,
            .sides_right=0b000,
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
            .sides_left=0b001,
            .sides_right=0b000,
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
            .sides_left=0b000,
            .sides_right=0b000,
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
            .sides_left=0b000,
            .sides_right=0b001,
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
            .sides_left=0b000,
            .sides_right=0b010,
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
            .sides_left=0b000,
            .sides_right=0b100,
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
            .sides_left=0b000,
            .sides_right=0b000,
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
            .sides_left=0b000,
            .sides_right=0b000,
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
