/*
 * I/O user interface
 *
 * This file provides functions to provide a basic user interface using the
 * Arduino UNO LED (L) a push button connected beween pins 12 and GND, and
 * an optional buzzer connected between pins 2 and GND.
 *
 * The two other LEDs on the UNO board (RX/TX) are only accessible by the
 * USB interface and can be set using the automation API (see automation.h)
 */

#ifndef USER_IO_H
#define USER_IO_H

#include <stdint.h>
#include <stdbool.h>

/* Button minimum hold time (ms) -- avoid counting bounces as presses */
#define BUTTON_HOLD_TIME_MS 20

/* Indicates how long we wait before sending another beep/blink */
#define DELAY_BETWEEN_BLINKS_MS 50
/* Indicates how long the led should stay on */
#define DELAY_BETWEEN_BLINK_SIGNALS 25

/*
DDRx - "Data Direction Register": Legt die Richtung der Daten fest, d.h. entweder lesen oder schreiben
PORTx - "Port Data Register": liest bzw. schreibt Daten vom bzw. zum Port
PINx - "Port Input Pins Register" liegt Daten vom Port
*/
// Port D – zur Ansteuerung der digitalen Pins 0 bis 7
// 			7 6 5 4 3 2  1  0
//			7 6 5 4 3 2 TX RX
// Port B – zur Ansteuerung der digitalen Pins 8 bis 13
// 			7 6  5  4  3  2 1 0
//			x x 13 12 11 10 9 8
// Port C – zur Ansteuerung der analogen Pins 0 bis 5
// https://elektro.turanis.de/html/prj129/index.html

enum pins_t {
	/* PORTC */


	/* PORTB */
	PIN_INPUT_TYPE 		= (1 << 0), 	/* Input type on pin 8 port B */
	PIN_BUZZER 			= (1 << 2), 	/* Buzzer on pin 10 port B */
	PIN_LED 			= (1 << 5), 	/* LED on pin 13 port B */

	/* PORTD */
	PIN_MAIN_BUTTON 	= (1 << 2), 	/* Button on pin 2 port D */
	PIN_TEST_BUTTON 	= (1 << 3), 	/* Button on pin 3 port D */
	PIN_TEST_BUTTON2	= (1 << 4), 	/* Button on pin 4 port D */
	PIN_TEST_BUTTON3	= (1 << 5), 	/* Button on pin 5 port D */
	PIN_SWITCH_INPUT	= (1 << 6), 	/* Button on pin 6 port D */
};

enum pin_ports_t {
	_PORTC = 0,
	_PORTB = 1,
	_PORTD = 2
};

typedef struct pin_t {
  uint32_t pin; /* The pin 0000 0000 */
  enum pin_ports_t port; /* The port group of the pin, user-io.c */
  bool output; /* True for output false for input */
} pin_t;

extern pin_t led_pin;
extern pin_t buzzer_pin;

/*
 * Pin functions
 */
/*
 * For buttons, check wether the button is held or not 
 */
bool button_held(pin_t pin);

/*
 * Determins wether or not the pin should be HIGH or LOW
 */
void set_pin(pin_t pin, bool state);

/* Structure to track button presses */
typedef struct button_info {
	uint8_t hold_time; /* Time the button was held down */
	uint8_t count; /* Number of times the button was pressed */
	pin_t button; /* Pin number of the button to press */
} button_info;

/*
 * Initializes the IO interface. Must be called before calling other functions
 * in this file.
 */
void init_pin(pin_t pin);
/*
 * Wait the specified amount of time for the button to be pressed. If the
 * button is not pressed, this function returns false. If the button, is
 * pressed, this function waits for the button to be released, and returns
 * true.
 */
bool wait_for_button_timeout(uint16_t led_on_time_ms, uint16_t led_off_time_ms, uint16_t timeout_ms, pin_t button);

/*
 * Blink the LED and wait for the user to press the button. Return the number
 * of presses. The user is allowed 500 ms between button presses before this
 * function returns.
 */
uint8_t count_button_presses(uint16_t led_on_time_ms, uint16_t led_off_time_ms, pin_t button);

/*
 * Wait a fixed amount of time, blinking the LED (unless led_on_time_ms is 0).
 *
 * Returns the number of times the button was pressed.
 */
uint8_t delay(uint16_t led_on_time_ms, uint16_t led_off_time_ms, uint16_t delay_ms, pin_t button);

/*
 * Emit a brief beep the buzzer.
 */
void beep(uint8_t amt);


#endif
