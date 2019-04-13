#include <avr/io.h>
#include <avr/interrupt.h>

#include "animations.h"

// SIDES
#define SIDES_LEFT_PORT PORTD
#define SIDES_LEFT_DDR DDRD
#define SIDES_LEFT_SHIFT 5

#define SIDES_RIGHT_PORT PORTB
#define SIDES_RIGHT_DDR DDRB
#define SIDES_RIGHT_SHIFT 0

// MATRIX
#define MATRIX_PORT PORTD
#define MATRIX_DDR DDRD
#define MATRIX_EN PD0
#define MATRIX_DATA PD1
#define MATRIX_SHIFT PD2
#define MATRIX_STORE PD3

volatile Animation *activeAnimation = 0;
volatile uint8_t activeFrame;
volatile uint8_t activeRow;
volatile uint8_t frameExposure;

void sides_init() {
    SIDES_LEFT_DDR |= 0b111 << SIDES_LEFT_SHIFT;
    SIDES_LEFT_PORT &= ~(0b111 << SIDES_LEFT_SHIFT);

    SIDES_RIGHT_DDR |= 0b111 << SIDES_RIGHT_SHIFT;
    SIDES_RIGHT_PORT &= ~(0b111 << SIDES_RIGHT_SHIFT);
}

void matrix_init() {
    MATRIX_DDR |= 1 << MATRIX_EN | 1 << MATRIX_DATA | 1 << MATRIX_SHIFT | 1 << MATRIX_STORE;
    MATRIX_PORT &= ~(1 << MATRIX_EN | 1 << MATRIX_DATA | 1 << MATRIX_SHIFT | 1 << MATRIX_STORE);
}

void matrix_shift(uint8_t value) {
    for (int i = 0; i < 8; ++i) {
        if (value & (1 << i)) {
            MATRIX_PORT |= 1 << MATRIX_DATA;
        } else {
            MATRIX_PORT &= ~(1 << MATRIX_DATA);
        }
        MATRIX_PORT |= 1 << MATRIX_SHIFT;
        MATRIX_PORT &= ~(1 << MATRIX_SHIFT);
    }
}

void matrix_store() {
    MATRIX_PORT |= 1 << MATRIX_STORE;
    MATRIX_PORT &= ~(1 << MATRIX_STORE);
}

// SWITCH

#define SW_DDR DDRB
#define SW_INP PINB
#define SW_PORT PORTB
#define SW_INPUT_LEFT PB6
#define SW_INPUT_RIGHT PB7

enum {
    SW_POS_UNDEFINED,
    SW_POS_MIDDLE,
    SW_POS_LEFT,
    SW_POS_RIGHT
};

volatile uint8_t currentSwitch = SW_POS_UNDEFINED;

void sw_init() {
    SW_DDR &= ~(1 << SW_INPUT_LEFT | 1 << SW_INPUT_RIGHT);
    SW_PORT |= 1 << SW_INPUT_LEFT | 1 << SW_INPUT_RIGHT;
}

void sw_update() {
    uint8_t inp = SW_INP;
    if ((inp & (1 << SW_INPUT_LEFT)) == 0) {
        if (currentSwitch != SW_POS_LEFT) {
            currentSwitch = SW_POS_LEFT;

            activeAnimation = &turningLeft;
            activeFrame = 0;
            activeRow = 0;
            frameExposure = 0;
        }
    } else if ((inp & (1 << SW_INPUT_RIGHT)) == 0) {
        if (currentSwitch != SW_POS_RIGHT) {
            currentSwitch = SW_POS_RIGHT;

            activeAnimation = &turningRight;
            activeFrame = 0;
            activeRow = 0;
            frameExposure = 0;
        }
    } else if (currentSwitch != SW_POS_MIDDLE) {
        currentSwitch = SW_POS_MIDDLE;

        activeAnimation = &idle;
        activeFrame = 0;
        activeRow = 0;
        frameExposure = 0;
    }
}

void start_timer() {
    TCCR0A = 2; // WGM = CTC
    TCCR0B = 3; // prescaler: IOclk(1MHz) / 64 => 15625Hz
    OCR0A = 15; // 15625Hz / (15 + 1) => 976.5Hz
    // refresh rate: 976.5Hz / 9 rows => 108.5Hz

    // enable TIMER0_COMPA interrupt
    TIMSK0 = 1 << OCIE0A;
}

ISR(TIMER0_COMPA_vect) {
    sw_update();

    if (activeAnimation == 0) {
        return;
    }

    uint8_t hi = 0, lo = ~(activeAnimation->frames[activeFrame].rows[activeRow]) & 0b11111;
    if (activeRow > 0) {
        hi = 1 << (activeRow - 1);
    } else {
        lo |= 1 << 7;
    }

    matrix_shift(lo);
    matrix_shift(hi);
    matrix_store();

    SIDES_LEFT_PORT = (SIDES_LEFT_PORT & ~((uint8_t)0b111 << SIDES_LEFT_SHIFT)) | activeAnimation->frames[activeFrame].sides_left << SIDES_LEFT_SHIFT;
    SIDES_RIGHT_PORT = (SIDES_RIGHT_PORT & ~((uint8_t)0b111 << SIDES_RIGHT_SHIFT)) | activeAnimation->frames[activeFrame].sides_right << SIDES_RIGHT_SHIFT;

    if (++activeRow >= FRAME_ROWS) {
        activeRow = 0;

        if (++frameExposure >= activeAnimation->frames[activeFrame].duration) {
            frameExposure = 0;

            if (++activeFrame >= activeAnimation->count) {
                activeFrame = 0;
            }
        }
    }
}

int main(void) {
    sw_init();
    sides_init();
    matrix_init();

    activeAnimation = &idle;
    start_timer();

    // enable interrupts
    sei();

    for (;;) {}
}
