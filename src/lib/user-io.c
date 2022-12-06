#include "user-io.h"

#include <avr/io.h>
#include <util/delay.h>

pin_t  led_pin		= { PIN_LED,	_PORTB, true };
pin_t  buzzer_pin	= { PIN_BUZZER, _PORTB, true };

/* Static functions */
static void track_button(button_info* info);
static uint8_t get_tracked_presses(button_info* info);

/* Initialize the give pin */
void init_pin(pin_t pin)
{
	if (pin.port == _PORTC) {
		if (pin.output) {
			DDRC |= pin.pin;
			PORTC &= ~pin.pin;
		}
		else {
			DDRC &= ~pin.pin;
			PORTC |= pin.pin;
		}
	}
	else if (pin.port == _PORTB) {
		if (pin.output) {
			DDRB |= pin.pin;
			PORTB &= ~pin.pin;
		}
		else {
			DDRB &= ~pin.pin;
			PORTB |= pin.pin;
		}
	}
	else if (pin.port == _PORTD) {
		if (pin.output) {
			DDRD |= pin.pin;
			PORTD &= ~pin.pin;
		}
		else {
			DDRD &= ~pin.pin;
			PORTD |= pin.pin;
		}
	}
}

/* Wait the specified amount of time for the button to be pressed. */
bool wait_for_button_timeout(uint16_t led_on_time_ms, uint16_t led_off_time_ms, uint16_t timeout_ms, pin_t button)
{
	const uint16_t led_cycle_time_ms = led_on_time_ms + led_off_time_ms;
	uint16_t led_cycle_pos = 1;
	button_info info = {0, 0, button};

	while (timeout_ms > 0) {
		if (led_cycle_pos == 1) {
			set_pin(led_pin, true);
		} else if (led_cycle_pos == led_on_time_ms) {
			set_pin(led_pin, false);
		} else if (led_cycle_pos == led_cycle_time_ms) {
			led_cycle_pos = 0;
		}

		track_button(&info);

		if (info.count) {
			break;
		}

		timeout_ms -= 1;
		led_cycle_pos += 1;
	}

	/* Will wait for the button to be released */
	uint8_t presses = get_tracked_presses(&info);
	set_pin(led_pin, false);

	return presses > 0;
}


/* Blink the LED and wait for the user to press the button. */
uint8_t count_button_presses(uint16_t led_on_time_ms, uint16_t led_off_time_ms, pin_t button)
{
	const uint16_t led_cycle_time_ms = led_on_time_ms + led_off_time_ms;
	uint16_t led_cycle_pos = 1;
	button_info info = {0, 0, button};
	uint16_t timeout_ms = 0;

	while ((info.count == 0) || (timeout_ms > 0)) {
		if (led_cycle_pos == 1) {
			set_pin(led_pin, true);
		} else if (led_cycle_pos == led_on_time_ms) {
			set_pin(led_pin, false);
		} else if (led_cycle_pos == led_cycle_time_ms) {
			led_cycle_pos = 0;
		}

		track_button(&info);

		if (info.hold_time) {
			timeout_ms = 500;
		}

		timeout_ms -= 1;
		led_cycle_pos += 1;
	}

	set_pin(led_pin, false);

	/* Will wait for the button to be released */
	return get_tracked_presses(&info);
}

/* Wait a fixed amount of time, blinking the LED */
uint8_t delay(uint16_t led_on_time_ms, uint16_t led_off_time_ms, uint16_t delay_ms, pin_t button)
{
	uint16_t led_cycle_time_ms = led_on_time_ms + led_off_time_ms;
	uint16_t led_cycle_pos = 1;
	button_info info = {0, 0, button};
	uint16_t remaining = delay_ms;

	while (remaining > 0) {
		if (led_on_time_ms != 0) {
			if (led_cycle_pos == 1) {
				set_pin(led_pin, true);
			} else if (led_cycle_pos == led_on_time_ms) {
				set_pin(led_pin, false);
			} else if (led_cycle_pos == led_cycle_time_ms) {
				led_cycle_pos = 0;
			}
		}

		track_button(&info);

		remaining -= 1;
		led_cycle_pos += 1;
	}

	set_pin(led_pin, false);

	if (delay_ms <= BUTTON_HOLD_TIME_MS) {
		/* The wait delay is lower than the minimum hold time, so
		   get_tracked_presses will not return a correct number of press times.
		   Instead, we just return 1 if the button was held at all. */

		return (info.hold_time > 0) ? 1 : 0;
	}

	/* Will wait for the button to be released */
	return get_tracked_presses(&info);
}

/* Emit a brief beep the buzzer. */
void beep(uint8_t amt)
{
	for ( uint8_t i = 0; i < amt; i++) {
		set_pin(buzzer_pin, true);
		_delay_ms(DELAY_BETWEEN_BLINK_SIGNALS);
		set_pin(buzzer_pin, false);
		if (i != amt-1)
			_delay_ms(DELAY_BETWEEN_BLINKS_MS);
	}
}


/*
 * Track the button presses during roughly 1 ms.
 * The info struct must be initialized to all zeros before calling this
 * function.
 */
void track_button(button_info* info)
{
	if (button_held(info->button)) {
		/* The button is held; increment the hold time */
		info->hold_time += 1;

	} else {
		/* Check if the button was just released after being held for
		   a sufficient time */
		if (info->hold_time > BUTTON_HOLD_TIME_MS) {
			info->count += 1;
		}

		info->hold_time = 0;
	}

	_delay_ms(1);
}


/*
 * Count the button presses after a tracking operation.
 */
uint8_t get_tracked_presses(button_info* info)
{
	uint8_t count = info->count;

	/* Wait for the button to be released */
	while (button_held(info->button)) {
		/* Nothing */
	}

	/* Count the last button press (if the button was still held the last time
	   track_button was called */
	if (info->hold_time > BUTTON_HOLD_TIME_MS) {
		count += 1;
	}

	return count;
}


/*
 * For buttons, check wether the button is held or not 
 */
bool button_held(pin_t pin) {
  if (pin.port == _PORTC) {
    return (PINC & pin.pin) == 0;
  }
  else if (pin.port == _PORTB) {
    return (PINB & pin.pin) == 0;
  }
  else if (pin.port == _PORTD) {
    return (PIND & pin.pin) == 0;
  }
  return false;
}
/*
 * Determins wether or not the pin should be HIGH or LOW
 */
void set_pin(pin_t pin, bool state) {
  if (pin.port == _PORTC) {
    if (state) {
    	PORTC |= pin.pin;
    }
    else {
    	PORTC &= ~pin.pin;
    }
  }
  else if (pin.port == _PORTB) {
    if (state) {
    	PORTB |= pin.pin;
    }
    else {
    	PORTB &= ~pin.pin;
    }
  }
  else if (pin.port == _PORTD) {
    if (state) {
    	PORTD |= pin.pin;
    }
    else {
    	PORTD &= ~pin.pin;
    }
  }
}
