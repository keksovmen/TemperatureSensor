// ATtiny13 specific header
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
// #include <math.h>



// Define the One-Wire pin
#define ONEWIRE_PORT PORTB
#define ONEWIRE_PIN  PINB
#define ONEWIRE_DDR  DDRB

#define LED_A_PIN		PB0
#define LED_B_PIN		PB1
#define LED_C_PIN		PB2
#define LED_D_PIN		PB3
#define LED_ON_PIN		PB4

#define ONEWIRE_DQ   PB4



// Function Prototypes
static void onewire_init();
static bool onewire_reset();
static void onewire_write_bit(uint8_t bit);
static uint8_t onewire_read_bit();
static void onewire_write_byte(uint8_t byte);
static uint8_t onewire_read_byte();



static void onewire_init() {
    // Set the One-Wire pin as input
    ONEWIRE_DDR = ONEWIRE_DDR & ~(1 << ONEWIRE_DQ);
    // Enable pull-up resistor
    ONEWIRE_PORT = ONEWIRE_PORT | (1 << ONEWIRE_DQ);
}

static bool onewire_reset() {
    
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

static void onewire_write_bit(uint8_t bit) {
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

static uint8_t onewire_read_bit() {
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

static void onewire_write_byte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        onewire_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

static uint8_t onewire_read_byte() {
    uint8_t byte = 0;
    
    for (uint8_t i = 0; i < 8; i++) {
        byte >>= 1;
        if (onewire_read_bit()) {
            byte |= 0x80;
        }
    }
    
    return byte;
}

static short _val_to_port(short val)
{
	switch(val)
	{
		case 0:
			return (1 << LED_ON_PIN);

		case 1:
			return (1 << LED_ON_PIN) | (1 << LED_A_PIN);
		
		case 2:
			return (1 << LED_ON_PIN) | (1 << LED_B_PIN);
		
		case 3:
			return (1 << LED_ON_PIN) | (1 << LED_A_PIN) | (1 << LED_B_PIN);
		
		case 4:
			return (1 << LED_ON_PIN) | (1 << LED_C_PIN);
		
		case 5:
			return (1 << LED_ON_PIN) | (1 << LED_A_PIN) | (1 << LED_C_PIN);
		
		case 6:
			return (1 << LED_ON_PIN) | (1 << LED_B_PIN) | (1 << LED_C_PIN);
		
		case 7:
			return (1 << LED_ON_PIN) | (1 << LED_A_PIN) | (1 << LED_B_PIN) | (1 << LED_C_PIN);
		
		case 8:
			return (1 << LED_ON_PIN) | (1 << LED_D_PIN);
		
		case 9:
			return (1 << LED_ON_PIN) | (1 << LED_A_PIN) | (1 << LED_D_PIN);
	}

	return 0;
}

static void _turn_diod_on(short val)
{
	ONEWIRE_PORT = _val_to_port(val);
}

static void _turn_diod_off()
{
	ONEWIRE_PORT = ONEWIRE_PORT & ~(1 << LED_ON_PIN);
}



int main() {
	ONEWIRE_DDR = (1 << LED_ON_PIN) |
		(1 << LED_A_PIN) | (1 << LED_B_PIN) |
		(1 << LED_C_PIN) | (1 << LED_D_PIN);

    // Initialize One-Wire interface
    onewire_init();

	_delay_ms(200);

	// for(short i = 0; i <= 9; i++){
	// 	_turn_diod_on(i);
	// 	_delay_ms(500);

	// 	_turn_diod_off();
	// 	_delay_ms(300);
	// }

    // Example usage: Reset and check presence pulse
    if (onewire_reset()) {
        // Device is present, perform communication
        onewire_write_byte(0xCC); // Skip ROM command
        onewire_write_byte(0x44); // Convert T command for DS18B20
        _delay_ms(750); // Wait for conversion
        
        onewire_reset();
        onewire_write_byte(0xCC); // Skip ROM command
        onewire_write_byte(0xBE); // Read Scratchpad command
        
        uint8_t temp_lsb = onewire_read_byte();
        uint8_t temp_msb = onewire_read_byte();
        
        // Combine MSB and LSB to get the temperature
        int16_t temp = (temp_msb << 8) | temp_lsb;
        
        // Convert to Celsius (assuming 12-bit resolution)
        const int16_t temperature = abs(temp / 16);

		const short tens = temperature / 10;
		const short ones = temperature % 10;

		ONEWIRE_DDR = ONEWIRE_DDR | (1 << LED_ON_PIN);

		// if(temperature > 0){
		// 	_turn_minus_on();
		// 	_delay_ms(1000);
		// }

		_turn_diod_off();
		_delay_ms(500);

		if(tens > 0){
			_turn_diod_on(tens);
			_delay_ms(1000);

			_turn_diod_off();
			_delay_ms(500);
		}

		_turn_diod_on(ones);
		_delay_ms(1000);

		_turn_diod_off();
        // Use the temperature value as needed
    }else{
		ONEWIRE_DDR = ONEWIRE_DDR | (1 << LED_ON_PIN);

		for(short i = 0; i < 3; i++){
			_turn_diod_on(1);
			_delay_ms(500);

			_turn_diod_off();
			_delay_ms(500);
		}
	}
    return 0;
}