#include <stdbool.h>

#ifndef SIGNALS_SD_H
#define SIGNALS_SD_H

void spi_init();
bool sd_init();
void spi_enable_interrupt();
void spi_full_speed();
void sd_select();
void sd_unselect();
uint8_t spi_exchange(uint8_t out);
void spi_slow();
void spi_slow();

void sd_send_command8(uint8_t id, uint8_t arg[4]);
uint8_t sd_wait_response();

#endif //SIGNALS_SD_H
