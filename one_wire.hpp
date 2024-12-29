#pragma once



#include <avr/io.h>



// Define the One-Wire pin
#define ONEWIRE_PORT PORTB
#define ONEWIRE_PIN  PINB
#define ONEWIRE_DDR  DDRB
#define ONEWIRE_DQ   PB4



void onewire_init();
bool onewire_reset();
void onewire_write_bit(uint8_t bit);
uint8_t onewire_read_bit();
void onewire_write_byte(uint8_t byte);
uint8_t onewire_read_byte();