#include <avr/io.h>
#include <avr/interrupt.h>

#include "frame.h"
#include "animations.h"

// SIDES
#define SIDES_LEFT_PORT PORTB
#define SIDES_LEFT_DDR DDRB
#define SIDES_LEFT_SHIFT 0

#define SIDES_RIGHT_PORT PORTC
#define SIDES_RIGHT_DDR DDRC
#define SIDES_RIGHT_SHIFT 2

// MATRIX
#define MATRIX_PORT PORTD
#define MATRIX_DDR DDRD
#define MATRIX_EN 0
#define MATRIX_DATA 1
#define MATRIX_SHIFT 2
#define MATRIX_STORE 3

volatile Animation *activeAnimation = 0;
volatile uint8_t activeFrame;
volatile uint8_t activeRow;
volatile uint8_t frameExposure;

void sides_init() {
    uint8_t bits = 0b111 << SIDES_LEFT_SHIFT;
    SIDES_LEFT_DDR |= bits;
    SIDES_LEFT_PORT &= ~(bits);

    bits = 0b111 << SIDES_RIGHT_SHIFT;
    SIDES_RIGHT_DDR |= bits;
    SIDES_RIGHT_PORT &= ~(bits);
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

#define SW_DDR DDRD
#define SW_INP PIND
#define SW_PORT PORTD
#define SW_INPUT_LEFT 6
#define SW_INPUT_RIGHT 7

#define SW_POS_UNDEFINED 0
#define SW_POS_MIDDLE 1
#define SW_POS_LEFT 2
#define SW_POS_RIGHT 3

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

        activeAnimation = &dot;
        activeFrame = 0;
        activeRow = 0;
        frameExposure = 0;
    }
}

void start_timer() {
    TCCR0A = 2; // GWM = CTC
    TCCR0B = 3; // prescaler: IOclk(1MHz) / 64
    OCR0A = 17; // 920 Hz / 9 = 102 Hz / scan

    // enable TIMER0_COMPA interrupt
    TIMSK0 = 1 << OCIE0A;
    // enable interrupts
    sei();
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

    SIDES_LEFT_PORT = activeAnimation->frames[activeFrame].sides_left << SIDES_LEFT_SHIFT;
    SIDES_RIGHT_PORT = activeAnimation->frames[activeFrame].sides_right << SIDES_RIGHT_SHIFT;

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

    activeAnimation = &dot;
    start_timer();

    for (;;) {}
}
