#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdlib.h>

#include "one_wire.hpp"



#define _LED_PORT 		PORTB
#define _LED_DDR 		DDRB

#define _LED_A_PIN		PB0
#define _LED_B_PIN		PB1
#define _LED_C_PIN		PB2
#define _LED_D_PIN		PB3
#define _LED_ON_PIN		PB4

#define _TIME_ON_MS 500
#define _TIME_OFF_MS 100
#define _PRECISION_DELAY_MS 800



static uint8_t _val_to_port(uint8_t val)
{
	//compiles in to more commands than switch statement
	// static const uint8_t map[] = {
	// 	(1 << _LED_ON_PIN),
	// 	(1 << _LED_ON_PIN) | (1 << _LED_A_PIN),
	// 	(1 << _LED_ON_PIN) | (1 << _LED_B_PIN),
	// 	(1 << _LED_ON_PIN) | (1 << _LED_A_PIN) | (1 << _LED_B_PIN),
	// 	(1 << _LED_ON_PIN) | (1 << _LED_C_PIN),
	// 	(1 << _LED_ON_PIN) | (1 << _LED_A_PIN) | (1 << _LED_C_PIN),
	// 	(1 << _LED_ON_PIN) | (1 << _LED_B_PIN) | (1 << _LED_C_PIN),
	// 	(1 << _LED_ON_PIN) | (1 << _LED_A_PIN) | (1 << _LED_B_PIN) | (1 << _LED_C_PIN),
	// 	(1 << _LED_ON_PIN) | (1 << _LED_D_PIN),
	// 	(1 << _LED_ON_PIN) | (1 << _LED_A_PIN) | (1 << _LED_D_PIN)
	// };

	// return map[val];

	switch(val)
	{
		case 0:
			return (1 << _LED_ON_PIN);

		case 1:
			return (1 << _LED_ON_PIN) | (1 << _LED_A_PIN);
		
		case 2:
			return (1 << _LED_ON_PIN) | (1 << _LED_B_PIN);
		
		case 3:
			return (1 << _LED_ON_PIN) | (1 << _LED_A_PIN) | (1 << _LED_B_PIN);
		
		case 4:
			return (1 << _LED_ON_PIN) | (1 << _LED_C_PIN);
		
		case 5:
			return (1 << _LED_ON_PIN) | (1 << _LED_A_PIN) | (1 << _LED_C_PIN);
		
		case 6:
			return (1 << _LED_ON_PIN) | (1 << _LED_B_PIN) | (1 << _LED_C_PIN);
		
		case 7:
			return (1 << _LED_ON_PIN) | (1 << _LED_A_PIN) | (1 << _LED_B_PIN) | (1 << _LED_C_PIN);
		
		case 8:
			return (1 << _LED_ON_PIN) | (1 << _LED_D_PIN);
		
		case 9:
			return (1 << _LED_ON_PIN) | (1 << _LED_A_PIN) | (1 << _LED_D_PIN);
		
		default:
			return 0;
	}
}

static uint8_t _precision_to_value(uint8_t precision)
{
	const static uint8_t map[] = {0, 25, 50, 75};

	return map[precision];

	//compiles in to more commands
	// switch (precision)
	// {
	// 	case 0:
	// 		return 0;
		
	// 	case 1:
	// 		return 25;
		
	// 	case 2:
	// 		return 50;
		
	// 	case 3:
	// 		return 75;

	// 	default:
	// 		return 0;;
	// }
}



static void _turn_diod_on(uint8_t val)
{
	_LED_PORT = _val_to_port(val);
}

static void _turn_diod_off()
{
	_LED_DDR = _LED_DDR | (1 << _LED_ON_PIN);
	_LED_PORT = _LED_PORT & ~(1 << _LED_ON_PIN);
}



static void _show_error(uint8_t error)
{
	for(uint8_t i = 0; i < 3; i++){
		_turn_diod_on(error);
		_delay_ms(_TIME_ON_MS);

		_turn_diod_off();
		_delay_ms(_TIME_ON_MS);
	}
}

static void _show_number(int16_t number)
{
	const uint8_t tens = number / 10;
	const uint8_t ones = number % 10;

	if(tens > 0){
		_turn_diod_on(tens);
		_delay_ms(_TIME_ON_MS);

		_turn_diod_off();
		_delay_ms(_TIME_OFF_MS);
	}

	_turn_diod_on(ones);
	_delay_ms(_TIME_ON_MS);

	_turn_diod_off();
	_delay_ms(_TIME_OFF_MS);
}

static void _show_temperature(int16_t temperature, uint8_t precision)
{
	//show negative sign
	if(temperature < 0){
		for(uint8_t i = 0; i < 5; i++){
			_turn_diod_on(0);
			_delay_ms(100);

			_turn_diod_off();
			_delay_ms(100);
		}
	}

	//wait some time for beginning of show case
	_turn_diod_off();
	_delay_ms(300);

	//show main temperature
	_show_number(abs(temperature));

	if(precision > 0){
		//wait some delay for displaying next section
		_delay_ms(_PRECISION_DELAY_MS);
		_show_number(_precision_to_value(precision));
	}
}



static void _initialize()
{
	//set display outputs
	_LED_DDR = (1 << _LED_A_PIN) | (1 << _LED_B_PIN) |
		(1 << _LED_C_PIN) | (1 << _LED_D_PIN);

	_turn_diod_off();

	//wait for DS18B20 startup
	_delay_ms(200);

	// Initialize One-Wire interface
	onewire_init();
}



int main()
{
	_initialize();

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

        // Convert to Celsius (assuming 12-bit resolution)
        const int16_t temperature = combined >> 4;

		// get 0.25 +- precision
		uint8_t precision = temp_lsb & 0x0F;
		if(combined < 0){
			precision = ~(precision - 1);
			precision = precision & 0x0F;
		}
		precision = precision >> 2;

		//for debugging
		if(precision > 3){
			_show_error(7);
		}else{
			_show_temperature(temperature, precision);
		}
    }
	else
	{
		for(uint8_t i = 0; i < 10; i++){
			_turn_diod_on(i);
			_delay_ms(_TIME_ON_MS);

			_turn_diod_off();
			_delay_ms(_TIME_ON_MS);
		}
	}

	_turn_diod_off();

	//go to sleep for power save
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_mode();

    return 0;
}