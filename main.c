#include <avr/interrupt.h>
#include "sd.h"

// ADC references and inputs
#define ADMUX_REFS_INTERNAL11V ((uint8_t)0b11)
#define ADMUX_MUX_ACCX ((uint8_t)0b0111)
#define ADMUX_MUX_ACCY ((uint8_t)0b0000)
#define ADMUX_MUX_ACCZ ((uint8_t)0b0001)
// ADC prescaler
#define ADCSRA_ADPS_64 ((uint8_t)0b110)

/*
 *    -1g     0g    +1g
 * == 475 == 690 == 913 ==> X
 * == 490 == 705 == 914 ==> Y
 * == 492 == 720 == 922 ==> Z
 */

#define RING_CAPACITY 512

volatile struct {
    enum {
        IDLE,
        MEASURING_X,
        MEASURING_Y,
        MEASURING_Z
    } adc;
    struct {
        uint8_t data[RING_CAPACITY];
        uint16_t size;
        uint16_t next_write;
        uint16_t next_read;
    } ring;
    enum {
        TX_START, // sending multi-block data token (0xfc)
        TX_PROGRESS, // sending ring contents
        TX_EMPTY, // sending ring contents
        TX_CRC, // sending 2 fake crc16 bytes
        PRG_WAITING, // waiting for 0xff response
    } sd;
    uint16_t sd_tx_count;
} state;

uint8_t ring_read() {
    uint8_t r = state.ring.data[state.ring.next_read];
    state.ring.size--;
    if (++state.ring.next_read >= RING_CAPACITY) state.ring.next_read = 0;
    return r;
}

void tx_progress(uint8_t byte) {
    SPDR0 = byte;
    if (++state.sd_tx_count >= 512) {
        state.sd_tx_count = 0;
        state.sd = TX_CRC;
    }
}

void ring_add(uint8_t byte) {
    PORTD ^= 1 << PD1;
    if (state.sd == TX_EMPTY) {
        state.sd = TX_PROGRESS;
        tx_progress(byte);
        return;
    }

    state.ring.data[state.ring.next_write] = byte;
    state.ring.size++;
    if (++state.ring.next_write >= RING_CAPACITY) state.ring.next_write = 0;
}

void state_init() {
    state.adc = IDLE;
    state.ring.size = 0;
    state.ring.next_write = 0;
    state.ring.next_read = 0;
}

void adc_init() {
    // prescaler: 8Mhz / 64 = 125kHz
    ADCSRA = 1 << ADEN | 1 << ADIE | ADCSRA_ADPS_64 << ADPS0;
}

void timer_init() {
    // CTC
    TCCR0A = 0b10;
    // 7812.5Hz * 78 ~= 10ms
    OCR0A = 78;
    // 8Mhz / 1024 = 7812.5Hz
    TCCR0B = 0b101;
    // enable "compare A" interrupt
    TIMSK0 |= 1 << OCIE0A;
}

void adc_start(uint8_t input) {
    // choose input
    ADMUX = ADMUX_REFS_INTERNAL11V << REFS0 | input << MUX0;
    // start conversion
    ADCSRA |= 1 << ADSC;
}

int main() {
    DDRD = 1 << PD1 | 1 << PD2 | 1 << PD3;
    state_init();
    adc_init();
    spi_init();
    if (!sd_init()) return 1;

    // multiple block write starting at 0x02d00000 (sector 0x16800)
    sd_send_command8(25, (uint8_t[4]) {0, 1, 0x68, 0});
    if (sd_wait_response() != 0) return 1;

    sei();
    spi_enable_interrupt();

    state.sd = TX_START;
    SPDR0 = 0xff;

    timer_init();
    for (;;);
}

// ADC conversion complete
ISR(ADC_vect) {
    // ADCL should be read first
    uint8_t low = ADCL;
    ring_add(ADCH);
    ring_add(low);

    switch (state.adc) {
        case MEASURING_X:
            state.adc = MEASURING_Y;
            adc_start(ADMUX_MUX_ACCY);
            break;
        case MEASURING_Y:
            state.adc = MEASURING_Z;
            adc_start(ADMUX_MUX_ACCZ);
            break;
        case MEASURING_Z:
            state.adc = IDLE;
            break;
        default:
            break;
    }
}

ISR(TIMER0_COMPA_vect) {
    // pad with 0 zeros
    ring_add(0);
    ring_add(0);
    state.adc = MEASURING_X;
    adc_start(ADMUX_MUX_ACCX);
}

ISR(SPI0_STC_vect) {
    switch (state.sd) {
        case TX_START:
            state.sd = TX_PROGRESS;
            state.sd_tx_count = 0;
            SPDR0 = 0xfc;
            break;
        case TX_PROGRESS:
            if (!state.ring.size) {
                state.sd = TX_EMPTY;
                return;
            }
            tx_progress(ring_read());
            break;
        case TX_CRC:
            if (state.sd_tx_count++ < 2) {
                SPDR0 = 0x55;
                return;
            }
            spi_slow();
            SPDR0 = 0xff;
            state.sd = PRG_WAITING;
            break;
        case PRG_WAITING:
            if (SPDR0 == 0xff) {
                spi_full_speed();
                state.sd = TX_START;
            }
            SPDR0 = 0xff;
            break;
        default:
            break;
    }
}
