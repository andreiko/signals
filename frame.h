#ifndef SIGNALS_FRAME_H
#define SIGNALS_FRAME_H

#include <stdint.h>

#define FRAME_ROWS 9

typedef struct {
    uint8_t rows[FRAME_ROWS];
    uint8_t duration;
    uint8_t sides;
} Frame;

typedef struct {
    uint8_t count;
    Frame *frames;
} Animation;

#endif //SIGNALS_FRAME_H
