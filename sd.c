#include <avr/io.h>
#include <stdbool.h>

#include "sd.h"

#define SPI_DDR DDRB
#define SPI_SCK PB5
#define SPI_MISO PB4
#define SPI_MOSI PB3
#define SPI_SS PB2

#define SD_CS_DDR DDRD
#define SD_CS_PORT PORTD
#define SD_CS_PIN PD0

bool sd_init() {
    for (;;) {
        // send 10 blank cycles to get the card controller going
        sd_unselect();
        for (uint8_t i = 0; i < 10; i++) spi_exchange(0xff);

        // activate line
        sd_select();

        // GO_IDLE_STATE(0)
        spi_exchange(0xff);
        sd_send_command8(0, (uint8_t[4]) {0, 0, 0, 0});
        if (sd_wait_response() == 1) break;
    }

    // SEND_IF_COND(0)
    spi_exchange(0xff);
    sd_send_command8(8, (uint8_t[4]) {0, 0, 0, 0});
    if (sd_wait_response() != 1) return false;

    // skip 4+1 cycles
    for (uint8_t i = 0; i < 5; i++) spi_exchange(0xff);

    for (;;) {
        // APP_CMD(0)
        spi_exchange(0xff);
        sd_send_command8(55, (uint8_t[4]) {0, 0, 0, 0});
        if (sd_wait_response() != 1) return false;

        // SD_SEND_OP_COND(0x40000000)
        spi_exchange(0xff);
        sd_send_command8(41, (uint8_t[4]) {0x40, 0, 0, 0});
        if (sd_wait_response() == 0) break;
    }

    // skip one cycle
    spi_exchange(0xff);

    spi_full_speed();

    return true;
}

void sd_send_command8(uint8_t id, uint8_t arg[4]) {
    // command: 01<id:6>
    spi_exchange((id & (uint8_t) 0x3f) | (uint8_t) 0x40);

    switch(id) {
        case 0:
            for (uint8_t i = 0; i < 4; i++) spi_exchange(0);
            // canned crc7 for GO_IDLE_STATE(0) as xxxxxxx1
            spi_exchange((0x4a << 1) | 1);
            break;
        case 8:
            spi_exchange(1);
            spi_exchange(2);
            spi_exchange(3);
            spi_exchange(4);
            // canned crc7 for SEND_IF_COND(0x1020304) as xxxxxxx1
            spi_exchange((5 << 1) | 1);
            break;
        default:
            for (uint8_t i = 0; i < 4; i++) spi_exchange(arg[i]);
            spi_exchange(0x55);
    }
}

void sd_send_command32(uint8_t id, uint32_t arg) {
    sd_send_command8(id, (uint8_t *) (&arg));
}

uint8_t sd_wait_response() {
    for (uint16_t i = 0; i < 0xffff; i++) {
        uint8_t in = spi_exchange(0xff);
        if (in < 0x80) {
            return in;
        }
    }
    return 0xff;
}

void spi_init() {
    SD_CS_DDR |= 1 << SD_CS_PIN;
    SPI_DDR |= 1 << SPI_SCK | 1 << SPI_MOSI | 1 << SPI_SS;
    SPCR0 |= 1 << SPE | 1 << MSTR | 1 << SPR1;
}

void spi_enable_interrupt() {
    SPCR0 |= 1 << SPIE;
}

void spi_full_speed() {
    SPCR0 &= ~(1 << SPR1 | 1 << SPR0);
    SPSR0 |= 1 << SPI2X;
}

void spi_slow() {
    SPCR0 |= 1 << SPR1;
    SPSR0 &= ~(1 << SPI2X);
}

void sd_select() {
    SD_CS_PORT &= ~(1 << SD_CS_PIN);
}

void sd_unselect() {
    SD_CS_PORT |= 1 << SD_CS_PIN;
}

uint8_t spi_exchange(uint8_t out) {
    SPDR0 = out;
    while (!(SPSR0 & (1 << SPIF)));
    return SPDR0;
}
