#include <avr/interrupt.h>

// ADC references and inputs
#define ADMUX_REFS_INTERNAL11V ((uint8_t)0b11)
#define ADMUX_MUX_ACCX ((uint8_t)0b0111)
#define ADMUX_MUX_ACCY ((uint8_t)0b0000)
#define ADMUX_MUX_ACCZ ((uint8_t)0b0001)
// ADC prescaler
#define ADCSRA_ADPS_8 ((uint8_t)0b011)

/*
 *    -1g     0g    +1g
 * == 475 == 690 == 913 ==> X
 * == 490 == 705 == 914 ==> Y
 * == 492 == 720 == 922 ==> Z
 */

int16_t scale_to_mg(int16_t value, int16_t negative, int16_t zero, int16_t positive) {
    if (value > zero) {
        int32_t scale = ((int32_t)positive - zero);
        int32_t v = ((int32_t)value - zero) * 1000;
        return (int16_t)(v / scale);
    } else  if (value < zero) {
        int32_t scale = ((int32_t)zero - negative);
        int32_t v = ((int32_t)value - zero) * 1000;
        return (int16_t)(v / scale);
    } else {
        return 0;
    }
}

int16_t x_to_mg(int16_t value) {
    return scale_to_mg(value, 475, 690, 913);
}

int16_t y_to_mg(int16_t value) {
    return scale_to_mg(value, 490, 705, 914);
}

int16_t z_to_mg(int16_t value) {
    return scale_to_mg(value, 492, 720, 922);
}

uint8_t format_int16(int16_t input, volatile uint8_t *output) {
    uint8_t written = 0;
    int16_t start = 10000;
    if (input < 0) {
        start = -10000;
        output[written++] = '-';
    }

    for (int16_t i = start; i != 0; i = i / (int16_t) 10) {
        int8_t d = (int8_t) (input / i);
        if (d > 0 || written > 1 || (written > 0 && output[0] != '-')) {
            output[written++] = (uint8_t) '0' + d;
        }
        input = input % i;
    }

    if (written < 1 || (written < 2 && output[0] == '-')) {
        output[written++] = '0';
    }

    return written;
}

volatile struct {
    enum {
        SLEEPING,
        MEASURING_X,
        MEASURING_Y,
        MEASURING_Z,
        SENDING
    } id;
    struct {
        uint16_t x;
        uint16_t y;
        uint16_t z;
    } ring[32];
    uint8_t ringpos;

    uint8_t buffer[24];
    uint8_t bufpos;
    uint8_t slept;
} state;

void state_init() {
    state.id = SLEEPING;
    for (uint8_t i = 0; i < 32; i++) {
        state.ring[i].x = 0;
        state.ring[i].y = 0;
        state.ring[i].z = 0;
    }
    state.ringpos = 0;
}

void uart_init() {
    const uint16_t baud_setting = (F_CPU / 4 / 9600 - 1) / 2;;
    UBRR0H = (uint8_t) (baud_setting >> 8);
    UBRR0L = (uint8_t) baud_setting;
    UCSR0A |= 1 << U2X0;
    UCSR0B |= 1 << TXEN0;
}

void adc_init() {
    // prescaler: 1Mhz / 8 = 125kHz
    ADCSRA = 1 << ADEN | 1 << ADIE | ADCSRA_ADPS_8 << ADPS0;
}

void timer_init() {
    // CTC
    TCCR0A = 0b10;
    // (1 / 7812.5Hz) * 156 => 20ms
    OCR0A = 255;
    // 8Mhz / 1024 = 7812.5Hz
    TCCR0B = 0b101;

    TIMSK0 |= 1 << OCIE0A;
}

void adc_start(uint8_t input) {
    // choose input
    ADMUX = ADMUX_REFS_INTERNAL11V << REFS0 | input << MUX0;
    // start conversion
    ADCSRA |= 1 << ADSC;
}

int main() {
    state_init();
    adc_init();
    uart_init();
    timer_init();
    sei();
    for (;;);
}

ISR(USART0_TX_vect) {
    state.id = SLEEPING;
}

ISR(USART0_UDRE_vect) {
    if (state.buffer[state.bufpos] != 0) {
        UDR0 = state.buffer[state.bufpos++];
    } else {
        UCSR0B &= ~(1 << UDRIE0);
    }
}

// ADC conversion complete
ISR(ADC_vect) {
    uint8_t low = ADCL;
    uint16_t result = (ADCH << 8) + low;

    switch (state.id) {
        case MEASURING_X:
            state.ring[state.ringpos].x = result;
            state.id = MEASURING_Y;
            adc_start(ADMUX_MUX_ACCY);
            break;
        case MEASURING_Y:
            state.ring[state.ringpos].y = result;
            state.id = MEASURING_Z;
            adc_start(ADMUX_MUX_ACCZ);
            break;
        case MEASURING_Z:
            state.ring[state.ringpos++].z = result;
            if (state.ringpos == 32) {
                state.ringpos = 0;
            }
            state.id = SENDING;

            uint32_t total = 0;
            for (uint8_t i = 0; i < 32; i++) {
                total += state.ring[i].x;
            }
            total /= 32;
            state.bufpos = format_int16(x_to_mg((int16_t)total), state.buffer);
            state.buffer[state.bufpos++] = ',';

            total = 0;
            for (uint8_t i = 0; i < 32; i++) {
                total += state.ring[i].y;
            }
            total /= 32;
            state.bufpos += format_int16(y_to_mg((int16_t)total), state.buffer + state.bufpos);
            state.buffer[state.bufpos++] = ',';

            total = 0;
            for (uint8_t i = 0; i < 32; i++) {
                total += state.ring[i].z;
            }
            total /= 32;
            state.bufpos += format_int16(z_to_mg((int16_t)total), state.buffer + state.bufpos);
            state.buffer[state.bufpos++] = '\n';
            state.buffer[state.bufpos++] = 0;

            state.bufpos = 0;

            // enable UART interrupts
            UCSR0B |= 1 << TXCIE0 | 1 << UDRIE0;

            break;
        default:
            break;
    }
}

ISR(TIMER0_COMPA_vect) {
    state.id = MEASURING_X;
    adc_start(ADMUX_MUX_ACCX);
}
