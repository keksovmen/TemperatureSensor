#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdlib.h>

#include "one_wire.hpp"



#define LED_A_PIN		PB0
#define LED_B_PIN		PB1
#define LED_C_PIN		PB2
#define LED_D_PIN		PB3
#define LED_ON_PIN		PB4

#define _NUMBER_ON_MS 500
#define _NUMBER_OFF_MS 100
#define _PRECISION_DELAY_MS 800



static uint8_t _val_to_port(uint8_t val)
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

static void _turn_diod_on(uint8_t val)
{
	ONEWIRE_PORT = _val_to_port(val);
}

static void _turn_diod_off()
{
	ONEWIRE_DDR = ONEWIRE_DDR | (1 << LED_ON_PIN);
	ONEWIRE_PORT = ONEWIRE_PORT & ~(1 << LED_ON_PIN);
}



static void _show_number(int16_t number)
{
	const uint8_t tens = number / 10;
	const uint8_t ones = number % 10;

	if(tens > 0){
		_turn_diod_on(tens);
		_delay_ms(_NUMBER_ON_MS);

		_turn_diod_off();
		_delay_ms(_NUMBER_OFF_MS);
	}

	_turn_diod_on(ones);
	_delay_ms(_NUMBER_ON_MS);

	_turn_diod_off();
	_delay_ms(_NUMBER_OFF_MS);
}

static void _show_temperature(int16_t temperature, uint8_t precision)
{
	const uint8_t map[] = {0, 25, 50, 75};
	
		// if(temperature > 0){
		// 	_turn_minus_on();
		// 	_delay_ms(1000);
		// }

	_turn_diod_off();
	_delay_ms(300);

	_show_number(temperature);

	if(precision > 0){
		_delay_ms(_PRECISION_DELAY_MS);
		_show_number(map[precision]);
	}
}

static void _show_error(uint8_t error)
{
	for(uint8_t i = 0; i < 3; i++){
		_turn_diod_on(error);
		_delay_ms(_NUMBER_ON_MS);

		_turn_diod_off();
		_delay_ms(_NUMBER_ON_MS);
	}
}



int main() {
	ONEWIRE_DDR = (1 << LED_A_PIN) | (1 << LED_B_PIN) |
		(1 << LED_C_PIN) | (1 << LED_D_PIN);

	_turn_diod_off();

	_delay_ms(200);

	// Initialize One-Wire interface
	onewire_init();

    // Example usage: Reset and check presence pulse
    if (onewire_reset()) {
        // Device is present, perform communication
        onewire_write_byte(0xCC); // Skip ROM command
        onewire_write_byte(0x44); // Convert T command for DS18B20

		_turn_diod_off();
        _delay_ms(750); // Wait for conversion
		onewire_init();
        
        onewire_reset();
        onewire_write_byte(0xCC); // Skip ROM command
        onewire_write_byte(0xBE); // Read Scratchpad command
        
        const uint8_t temp_lsb = onewire_read_byte();
        const uint8_t temp_msb = onewire_read_byte();
        
        // Combine MSB and LSB to get the temperature
        const int16_t combined = (temp_msb << 8) | temp_lsb;
		// get 0.25 +- precision
		uint8_t precision = temp_lsb & 0x0F;
		if(combined < 0){
			precision = ~(precision - 1);
		}
		precision = precision >> 2;
        
        // Convert to Celsius (assuming 12-bit resolution)
        const int16_t temperature = abs(combined >> 4);

		_show_temperature(temperature, precision);
    }
	else
	{
		_show_error(1);
	}

	_turn_diod_off();

	//go to sleep for power save
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_mode();

    return 0;
}