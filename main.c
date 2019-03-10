#include <avr/interrupt.h>

// ADC references and inputs
#define ADMUX_REFS_INTERNAL11V ((uint8_t)0b11)
#define ADMUX_MUX_ACCX ((uint8_t)0b0111)
#define ADMUX_MUX_ACCY ((uint8_t)0b0000)
#define ADMUX_MUX_ACCZ ((uint8_t)0b0001)
// ADC prescaler
#define ADCSRA_ADPS_8 ((uint8_t)0b011)

uint8_t format_uint16(uint16_t input, volatile uint8_t *output) {
    uint8_t written = 0;
    for (uint16_t i = 10000; i > 0; i = i / (uint16_t) 10) {
        uint8_t d = (uint8_t) (input / i);
        if (d > 0 || written > 0) {
            output[written++] = (uint8_t) '0' + d;
        }
        input = input % i;
    }

    if (!written) {
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
    } data;
    uint8_t buffer[16];
    uint8_t pos;
    uint8_t slept;
} state;

void state_init() {
    state.id = SLEEPING;
    state.data.x = 0;
    state.data.y = 0;
    state.data.z = 0;
    state.pos = 0;
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
    OCR0A = 156;
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
    if (state.buffer[state.pos] != 0) {
        UDR0 = state.buffer[state.pos++];
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
            state.data.x = result;
            state.id = MEASURING_Y;
            adc_start(ADMUX_MUX_ACCY);
            break;
        case MEASURING_Y:
            state.data.y = result;
            state.id = MEASURING_Z;
            adc_start(ADMUX_MUX_ACCZ);
            break;
        case MEASURING_Z:
            state.data.z = result;
            state.id = SENDING;

            state.pos = format_uint16(state.data.x, state.buffer);
            state.buffer[state.pos++] = ',';
            state.pos += format_uint16(state.data.y, state.buffer + state.pos);
            state.buffer[state.pos++] = ',';
            state.pos += format_uint16(state.data.z, state.buffer + state.pos);
            state.buffer[state.pos++] = '\n';
            state.buffer[state.pos] = 0;
            state.pos = 0;

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
