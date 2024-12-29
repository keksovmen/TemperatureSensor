#include "one_wire.hpp"

#include <util/delay.h>



void onewire_init()
{
	// Set the One-Wire pin as input
    ONEWIRE_DDR = ONEWIRE_DDR & ~(1 << ONEWIRE_DQ);
    // Enable pull-up resistor
    ONEWIRE_PORT = ONEWIRE_PORT | (1 << ONEWIRE_DQ);
}

bool onewire_reset()
{
	// Pull the line low for 480us
    ONEWIRE_DDR = ONEWIRE_DDR | (1 << ONEWIRE_DQ);
    ONEWIRE_PORT = ONEWIRE_PORT & ~(1 << ONEWIRE_DQ);
    _delay_us(480);
    
    // Release the line and wait for 70us
    ONEWIRE_DDR = ONEWIRE_DDR & ~(1 << ONEWIRE_DQ);
    _delay_us(70);
    
    // Read the presence pulse
    bool presence = (ONEWIRE_PIN & (1 << ONEWIRE_DQ)) == 0;
    
    // Wait for the remainder of the timeslot (410us)
    _delay_us(410);
    
    return presence;
}

void onewire_write_bit(uint8_t bit)
{
	// Pull the line low and hold for 1us
    ONEWIRE_DDR = ONEWIRE_DDR | (1 << ONEWIRE_DQ);
    ONEWIRE_PORT = ONEWIRE_PORT & ~(1 << ONEWIRE_DQ);
    _delay_us(1);
    
    if (bit) {
        // Release the line for a '1' bit
        ONEWIRE_DDR = ONEWIRE_DDR & ~(1 << ONEWIRE_DQ);
    }
    
    // Hold the line low for the duration of the timeslot (60us)
    _delay_us(60);
    
    // Release the line for recovery
    ONEWIRE_DDR = ONEWIRE_DDR & ~(1 << ONEWIRE_DQ);
    _delay_us(1);
}

uint8_t onewire_read_bit()
{
	uint8_t bit = 0;
    
    // Pull the line low for 1us
    ONEWIRE_DDR = ONEWIRE_DDR | (1 << ONEWIRE_DQ);
    ONEWIRE_PORT = ONEWIRE_PORT & ~(1 << ONEWIRE_DQ);
    _delay_us(1);
    
    // Release the line and wait for 14us
    ONEWIRE_DDR = ONEWIRE_DDR & ~(1 << ONEWIRE_DQ);
    _delay_us(14);
    
    // Read the bit value
    if (ONEWIRE_PIN & (1 << ONEWIRE_DQ)) {
        bit = 1;
    }
    
    // Wait for the remainder of the timeslot (45us)
    _delay_us(45);
    
    return bit;
}

void onewire_write_byte(uint8_t byte)
{
	for (uint8_t i = 0; i < 8; i++) {
        onewire_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

uint8_t onewire_read_byte()
{
	uint8_t byte = 0;
    
    for (uint8_t i = 0; i < 8; i++) {
        byte >>= 1;
        if (onewire_read_bit()) {
            byte |= 0x80;
        }
    }
    
    return byte;
}