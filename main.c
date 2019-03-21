#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// MATRIX
#define MATRIX_PORT PORTD
#define MATRIX_DDR DDRD
#define MATRIX_EN 0
#define MATRIX_DATA 1
#define MATRIX_SHIFT 2
#define MATRIX_STORE 3

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

void update(int8_t n) {
    switch(n) {
        case 0:
            matrix_shift(0b11111011);
            matrix_shift(0b00000000);
        break;
        case 1:
            matrix_shift(0b01111011);
            matrix_shift(0b00000001);
        break;
        case 2:
            matrix_shift(0b01111011);
            matrix_shift(0b00000010);
        break;
        case 3:
            matrix_shift(0b01111011);
            matrix_shift(0b00000100);
        break;
        case 4:
            matrix_shift(0b01111011);
            matrix_shift(0b00001000);
        break;
        case 5:
            matrix_shift(0b01111011);
            matrix_shift(0b00010000);
        break;
        case 6:
            matrix_shift(0b01111011);
            matrix_shift(0b00100000);
        break;
        case 7:
            matrix_shift(0b01111011);
            matrix_shift(0b01000000);
        break;
        case 8:
            matrix_shift(0b01111011);
            matrix_shift(0b10000000);
        break;
    }
    matrix_store();
}

int main(void) {
    sw_init();
    matrix_init();
    int8_t n = 4;
    update(n);

    for (;;) {
        uint8_t inp = SW_INP;
        if ((inp & (1 << SW_INPUT_LEFT)) == 0) {
            if (currentSwitch == SW_POS_RIGHT || currentSwitch == SW_POS_UNDEFINED) {
                currentSwitch = SW_POS_LEFT;
                if (++n > 8) {
                    n = 0;
                }
                update(n);
            }
        } else if ((inp & (1 << SW_INPUT_RIGHT)) == 0) {
            if (currentSwitch == SW_POS_LEFT || currentSwitch == SW_POS_UNDEFINED) {
                currentSwitch = SW_POS_RIGHT;
                if (++n > 8) {
                    n = 0;
                }
                update(n);
            }
        }
        _delay_ms(10);
    }
}
