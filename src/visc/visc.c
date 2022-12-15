#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "automation-utils.h"
#include "user-io.h"
#include "utils.h"

#define nullptr (void*)0

pin_t btn_refresh_dailies   = { PIN_MAIN_BUTTON,    _PORTD, false   };      // black wire
pin_t btn_make_sandwich     = { PIN_TEST_BUTTON,    _PORTD, false   };      // white wire
pin_t btn_buttons           = { PIN_TEST_BUTTON2,   _PORTD, false   };      // gray wire
pin_t btn_switch_to_virt    = { PIN_TEST_BUTTON3,   _PORTD, false   };      // purple wire
pin_t btn_switch_to_real    = { PIN_SWITCH_INPUT,   _PORTD, false   };      // blue wire
pin_t led_input_type        = { PIN_INPUT_TYPE,     _PORTB, true    };      // led

ulong_t lastSandwich = 0; 
ulong_t lastPickup = 0;


void output_selection(uint8_t presses) {
    for (uint8_t i = 0 ; i < presses ; i++) {
        beep(1);
        _delay_ms(200);
    }
}

// ITEM/EGG FARMING START
void make_egg_sandwich() {
    // interact with table
    SEND_BUTTON_SEQUENCE(
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1  },	
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	27 },
    );
    // choose "make a sandwhich"
    SEND_BUTTON_SEQUENCE(
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1  },	
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	100 },
    );
    // choose "great peanut butter sandwhich"
    // wait 8 seconds for the sandwhich to be made
    for (uint8_t i = 0; i < 7; i++) {
        SEND_BUTTON_SEQUENCE(
            { BT_NONE,  DP_DOWN,  SEQ_MASH, 1 },
            { BT_NONE,  DP_NEUTRAL, SEQ_HOLD, 3 }
        )
    }

    SEND_BUTTON_SEQUENCE(
        { BT_NONE,  DP_NEUTRAL, SEQ_HOLD,   5   },
        { BT_A,     DP_NEUTRAL, SEQ_MASH,   1   }, // choose sandwich
        { BT_NONE,  DP_NEUTRAL, SEQ_HOLD,   15  },
        { BT_A,     DP_NEUTRAL, SEQ_MASH,   1   }, // choose pick
        { BT_NONE,  DP_NEUTRAL, SEQ_HOLD,   200 }
    );


    // put ingredients on the bread
    //    - stick up 600ms, hold a, stick down 520ms, repeat 3 times, stay down
    for (uint8_t i = 0; i < 3; i++) {
        // move the left stick up for 15 cycles, aka 600ms
        // if we're on the second or third iteration
        // we only move the stick up 13 cycles, so that we dont go to far up
        for (uint8_t j = 0; j < (i != 0 ? 13 : 15); j++) {
            send_update(BT_NONE,	DP_NEUTRAL, S_TOP, S_NEUTRAL);
        }

        // press a to pickup the banana/ingredient
        // hold a and the left stick down to move the ingredient to the table
        for (uint8_t j = 0; j < 13; j++) {
            send_update(BT_A,	DP_NEUTRAL, S_BOTTOM, S_NEUTRAL);
        }

        // if its the first time, we drop the ingredient in the middle
        // if its the second time, we drop it slightly to the left
        // if its the third time we drop it slightly to the right

        if (i > 0) {
            // move left or right, depending on iteration
            for (uint8_t j = 0; j < 5; j++) {
                send_update(BT_A,	DP_NEUTRAL, i == 1 ? S_LEFT : S_RIGHT, S_NEUTRAL);
            }
        }
        // let go of a to drop the ingredient onto the sandwich
        send_update(BT_NONE,	DP_NEUTRAL, S_NEUTRAL, S_NEUTRAL);

        if (i > 0) {
            // move back to the middle
            for (uint8_t j = 0; j < 5; j++) {
                send_update(BT_NONE,	DP_NEUTRAL, i == 1 ? S_RIGHT : S_LEFT, S_NEUTRAL);
            }
        }
        // let go of a to drop the ingredient onto the sandwich
        send_update(BT_NONE,	DP_NEUTRAL, S_NEUTRAL, S_NEUTRAL);

        // 40 ms delay 
        SEND_BUTTON_SEQUENCE(
            { BT_NONE,  DP_NEUTRAL, SEQ_HOLD,   1   }
        );
    }

    beep(2);

    // finish the sandwich

    // we dropped all ingredients, now we need to wait a bit for the bread to popup
    // that takes roughly 1000ms, probably less
    _delay_ms(1500);

    beep(1);
    // then we mash a to drop the bread
    SEND_BUTTON_SEQUENCE(
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1  },	
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	37 },
    // then we mash a to drop the pick
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1  },	
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	37 },
    );
    beep(2);

    // and wait a bit for the animation to start
    // this takes around 8 seconds, starting when the pick dropped
    _delay_ms(8500);
    beep(1);

    // hit a to skip
    SEND_BUTTON_SEQUENCE(
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1  },	
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 },
    );
    beep(2);
    // wait a while again for the animation to finish and the stat change to popup
    // this takes around 25 seconds
    _delay_ms(25000);
    // hit a to continue
    SEND_BUTTON_SEQUENCE(
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1  },	
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 },
    );
}

void face_basket() {
    // turns the player left so it faces the basket
    send_update(BT_NONE, DP_NEUTRAL, S_LEFT, S_NEUTRAL);
    pause_automation();
}

void face_table() {
    // turns the player forward so it faces the table
    send_update(BT_NONE, DP_NEUTRAL, S_TOP, S_NEUTRAL);
    pause_automation();
}

void get_items_or_eggs_from_basket(bool facing_basket, bool get_eggs) {
    // if you look from the initial postion of when the player started the picnic,
    // the player needs to be on the left side of the basket, facing the the table

    // if we're not facing the basket, face it
    if (!facing_basket)
        face_basket();

    // we're trying to pick up eggs from the basket
    // we we press a once and then spam b
    // it turns out that if you press b on the "do you want to donate the egg" dialog
    // the game will put the egg in your inventory and not dontate it
    // so we can use the same code as above, just execute it a bunch more times
    // since there are 3? more dialogs that pop up

    // press a once to interact with the basket
    SEND_BUTTON_SEQUENCE(
        { BT_A,		DP_NEUTRAL,	SEQ_HOLD,	3 },	// fast forward dialog box
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
    );

    // then spam b for 75 cycles, aka 3 seconds or 125 cycles, aka 5 seconds, when we're trying to get eggs
    for (uint8_t i = 0; i < (get_eggs ? 125 : 75); i++) {
        SEND_BUTTON_SEQUENCE(
            { BT_B,		DP_NEUTRAL,	SEQ_HOLD,	3 },	// fast forward dialog box
            { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	2 }
        );
    } 
}
// ITEM/EGG FARMING END


// EGG HATCHING START
void open_box() {
    SEND_BUTTON_SEQUENCE(
        { BT_X,		DP_NEUTRAL,	SEQ_HOLD,	1  },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	20 },
        { BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	60 }
    );
    beep(1);
    // opening the box takes a few seconds
}

void exit_box() {
    SEND_BUTTON_SEQUENCE(
        { BT_B,		DP_NEUTRAL,	SEQ_HOLD,	1  },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	45 },
        { BT_B,		DP_NEUTRAL,	SEQ_HOLD,	1  },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	15 }
    );
    beep(1);
}

void reset_box_cursor() {
    // easiest way to reset the cursor is to just close and open the box
        SEND_BUTTON_SEQUENCE(
        { BT_B,		DP_NEUTRAL,	SEQ_HOLD,	1  },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	60 },
        { BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1  },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	60 }
    );
    beep(1);
}

void advance_box_if_necessary(uint8_t* column) {
    if (*column > 5) {
        SEND_BUTTON_SEQUENCE(
            { BT_R,     DP_NEUTRAL, SEQ_MASH,   1 },
            { BT_NONE,  DP_NEUTRAL, SEQ_HOLD,   10 },
        );
        *column = 0;
    }
}

void get_eggs_from_box(uint8_t* column) {
    /*
    - advance box if necessary
    - move cursor [column] times to the right
    - press minus to activate box selection
    - move cursor down 4 fields
    - press a to select 5 rows of eggs
    - move cursor 1 field down and [column] times to the left
    - press a to drop eggs in party
    */

    // advance box if necessary
   advance_box_if_necessary(column);

    // move cursor [column] times to the right
    for (uint8_t i = 0; i < *column; i++) {
        SEND_BUTTON_SEQUENCE(
            { BT_NONE,	DP_RIGHT,	SEQ_MASH,	1 },
            { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
        );
    }

    // press minus to activate box selection
    SEND_BUTTON_SEQUENCE(
        { BT_MINUS,	DP_NEUTRAL,	SEQ_MASH,	1 },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
    );

    // move cursor down 4 fields
    for (uint8_t i = 0; i < 4; i++) {
        SEND_BUTTON_SEQUENCE(
            { BT_NONE,	DP_DOWN,	SEQ_MASH,	1 },
            { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
        );
    }

    // press a to select 5 rows of eggs
    SEND_BUTTON_SEQUENCE(
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
    );

    // move cursor 1 field down and [column+1] times to the left
    SEND_BUTTON_SEQUENCE(
        { BT_NONE,	DP_DOWN,	SEQ_MASH,	1 },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
    );

    for (uint8_t i = 0; i < (*column+1); i++) {
        SEND_BUTTON_SEQUENCE(
            { BT_NONE,	DP_LEFT,	SEQ_MASH,	1 },
            { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
        );
    }

    // press a to drop eggs in party
    SEND_BUTTON_SEQUENCE(
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	5 }
    );
    
    beep(1);
}

void put_pokemons_in_box(uint8_t column) {
    /*
    - move cursor 1 field down and 1 field to the left
    - press minus to activate box selection
    - move cursor down 4 fields
    - press a to select 5 rows of pokemons
    - move cursor 1 field up and [column+1] times to the right
    - press a to drop pokemons in box
    */

    // move cursor 1 field down and 1 field to the left
    SEND_BUTTON_SEQUENCE(
        { BT_NONE,	DP_DOWN,	SEQ_MASH,	1 },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 },
        { BT_NONE,	DP_LEFT,	SEQ_MASH,	1 },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
    );

    // press minus to activate box selection
    SEND_BUTTON_SEQUENCE(
        { BT_MINUS,	DP_NEUTRAL,	SEQ_MASH,	1 },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
    );

    // move cursor down 4 fields
    for (uint8_t i = 0; i < 4; i++) {
        SEND_BUTTON_SEQUENCE(
            { BT_NONE,	DP_DOWN,	SEQ_MASH,	1 },
            { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
        );
    }

    // press a to select 5 rows of pokemons
    SEND_BUTTON_SEQUENCE(
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
    );

    // move cursor 1 field up and [column+1] times to the right
    SEND_BUTTON_SEQUENCE(
        { BT_NONE,	DP_UP,		SEQ_MASH,	1 },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
    );

    for (uint8_t i = 0; i < (column+1); i++) {
        SEND_BUTTON_SEQUENCE(
            { BT_NONE,	DP_RIGHT,	SEQ_MASH,	1 }
        );
    }

    // press a to drop pokemons in box
    SEND_BUTTON_SEQUENCE(
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1 },
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	5 }
    );

    beep(1);
}

void get_egg_cycles(uint16_t* hatch_time, uint16_t* wait_time) {
    /* Hatching time for each Egg cycles */
	const struct {
		uint16_t hatch_time; /* Time passed spinning for the Egg to hatch */
		uint16_t wait_time; /* Time passed spinning to get another Egg */
	} egg_cycles[] = {
		{ 900, 150 },	/*  5 Egg cycles, approx. 64 Eggs/hour */
		{ 1110,  0 },	/* 10 Egg cycles, approx. 60 Eggs/hour */
		{ 1650, 0 },	/* 15 Egg cycles, approx. 50 Eggs/hour */
		{ 1950, 0 },	/* 20 Egg cycles, approx. 40 Eggs/hour */
		{ 2300, 0 },	/* 25 Egg cycles, approx. 33 Eggs/hour */
		{ 2600, 0 },	/* 30 Egg cycles, approx. 30 Eggs/hour */
		{ 2900, 0 },	/* 35 Egg cycles, approx. 24 Eggs/hour */
		{ 3200, 0 },	/* 40 Egg cycles, approx. 22 Eggs/hour */
	};
    
    for (;;) {
		uint8_t cycle_idx = count_button_presses(500, 500, btn_buttons) - 1;

		if (cycle_idx < (sizeof(egg_cycles) / sizeof(*egg_cycles))) {
			/* Selection OK, beep once per press */
            output_selection(cycle_idx+1);

            if (hatch_time != nullptr)
                *hatch_time = egg_cycles[cycle_idx].hatch_time;

            if (wait_time != nullptr)
                *wait_time = egg_cycles[cycle_idx].wait_time;

			break;
		}

		/* Wrong selection */
		delay(100, 200, 1500, btn_buttons);
	}
}

bool hatch_egg(void)
{
	SEND_BUTTON_SEQUENCE(
		{ BT_B,		DP_NEUTRAL,	SEQ_MASH,	5 },	/* Validate Oh?” dialog */
	)

	/* Egg hatching animation */
	if (delay(250, 250, 14000, btn_buttons)) {
		return true;
	}

	SEND_BUTTON_SEQUENCE(
		{ BT_B,		DP_NEUTRAL,	SEQ_HOLD,	1  }, // x hatched
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	85 }, // wait for pokedex entry
		{ BT_B,		DP_NEUTRAL,	SEQ_HOLD,	1  }, // no nickname
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	85 }, // wait for close
	)

	return false;
}

void move_in_circles(uint16_t cycles)
{
	for (uint16_t i = 0 ; i < cycles ; i += 1) {
		send_update(BT_NONE,	DP_NEUTRAL, S_RIGHT, S_LEFT);
	}

	/* Reset sticks position */
	pause_automation();
}

void hatch_eggs(uint16_t hatch_time)
{
	/* Set the LEDs so the submenu is identifiable */
	set_leds(RX_LED);
	pause_automation();

    // reset leds
    PORTC = 0 & 0x3F;

    uint8_t hatchedEggs = 0;

    beep(1);
    move_in_circles(hatch_time);
    beep(1);

    // hatch the first egg
    if (hatch_egg()) {
        /* Operation stopped by the user */
        return;
    }

    // output to leds
    PORTC = ++hatchedEggs & 0x3F;

    // the first egg hatched
    // now we need to run 1/7 of the original time to hatch the next egg
    // we do this a total of 4 times to hatch the 4 remaining eggs
    for (uint8_t i = 0; i < 4; i++) {
        set_leds(TX_LED);
	    pause_automation();
        
        beep(2);
        move_in_circles(hatch_time / 7);
        beep(2);

        if (hatch_egg()) {
            /* Operation stopped by the user */
            break;
        }

        // output to leds
        PORTC = ++hatchedEggs & 0x3F;
    }
}

// EGG HATCHING END

int main() {

    init_millis(16000000UL); //frequency the atmega328p is running at
    sei();

    lastSandwich = millis(); 
    lastPickup   = millis();

	init_automation();    

    // for the binary counter
    // set the pins 0-5 to output
    DDRC |= 0B00111111;

	init_pin(led_pin);
	init_pin(buzzer_pin);

	init_pin(btn_refresh_dailies);
	init_pin(btn_switch_to_virt);
	init_pin(btn_buttons);
	init_pin(btn_make_sandwich);
    init_pin(btn_switch_to_real);
	init_pin(led_input_type);

	/* Initial beep to confirm that the buzzer works */
	beep(1);
    
    for (;;) { 
		set_leds(BOTH_LEDS);
        pause_automation();

        if (button_held(btn_refresh_dailies)) {
            beep(1);
            SEND_BUTTON_SEQUENCE(
                { BT_B,		DP_NEUTRAL,	SEQ_HOLD,	1 }
            );
            change_clock_day(false, 1);
            go_to_game();
            beep(1);
        }
        else if (button_held(btn_buttons)) {
            beep(3);
            uint8_t presses = count_button_presses(100, 100, btn_buttons);
            output_selection(presses);

            static uint8_t column = 0;
            if (presses == 1) {
                SEND_BUTTON_SEQUENCE(
                    { BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1 }
                );
            }
            else if (presses == 2) {
                SEND_BUTTON_SEQUENCE(
                    { BT_B,		DP_NEUTRAL,	SEQ_HOLD,	1 }
                );
            }
            else if (presses == 3) {
                SEND_BUTTON_SEQUENCE(
                    { BT_HOME,	DP_NEUTRAL,	SEQ_HOLD,	1 }
                );
            }

            else if (presses == 4) {
                // get eggs from box
                if (count_button_presses(250, 250, btn_buttons) == 1) {
                    open_box();
                }
                else {
                    exit_box();
                }
            }
            
            else if (presses == 5) {
                open_box();
                get_eggs_from_box(&column);
                column++;
                exit_box();
            }
            
            else if (presses == 6) {
                open_box();
                put_pokemons_in_box(column-1);
                exit_box();
            }
            
            else if (presses == 7) {
                static uint8_t column = 0;

                uint16_t hatch_time;
                get_egg_cycles(&hatch_time, nullptr);

                for (uint8_t i = 0; i < 6; i++) {

                    PORTC = (column+1) & 0x3F;

                    // only open the box if its the first time getting eggs from the box
                    // if we're getting eggs from the box again, we don't need to open it
                    // since we just resetted the cursor position to the first slot
                    if (i == 0) 
                        open_box();

                    get_eggs_from_box(&column);
                    exit_box();

                    // hatch eggs
                    //hatch_eggs(hatch_time);

                    open_box();
                    put_pokemons_in_box(column);
                    // we run this in a loop, so instead of closing the box and exiting the menu,
                    // just reset the cursor position and continue
                    reset_box_cursor();

                    // user canceled the operation
                    if (delay(100, 100, 2000, btn_buttons))
                        break;

                    column++;
                }

                PORTC = 0 & 0x3F;
            }

            else if (presses == 8) {
                static uint8_t column = 0;

                beep(1);
                uint16_t hatch_time;
                get_egg_cycles(&hatch_time, nullptr);

                beep(2);
                uint8_t times = count_button_presses(100, 100, btn_buttons);
                output_selection(times);

                for (uint8_t i = 0; i < times; i++) {
                    PORTC = (column+1) & 0x3F;

                    // only open the box if its the first time getting eggs from the box
                    // if we're getting eggs from the box again, we don't need to open it
                    // since we just resetted the cursor position to the first slot
                    if (i == 0) 
                        open_box();

                    get_eggs_from_box(&column);
                    exit_box();

                    // hatch eggs
                    hatch_eggs(hatch_time);                    

                    open_box();
                    put_pokemons_in_box(column);
                    // we run this in a loop, so instead of closing the box and exiting the menu,
                    // just reset the cursor position and continue
                    reset_box_cursor();

                    // user canceled the operation
                    if (delay(100, 100, 2000, btn_buttons))
                        break;

                    column++;
                }
            }
        }
        else if (button_held(btn_make_sandwich)) {
            beep(2);
            uint8_t mode = count_button_presses(500, 500, btn_make_sandwich);

            if (mode == 1) {
                set_leds(RX_LED);
                send_current();
                // make the sandwich
                make_egg_sandwich();
                beep(2);
            }
            else {
                // loop
                bool facingBasket = false;
                bool cancel = false;
                uint8_t sandwichesMade = 0;

                for (uint8_t k = 0; k < 12; k++) {
                    if (k < 6)
                        PORTC = 1 << k; 
                    else
                        PORTC = 1 << (11 - k);
                    _delay_ms(100);
                }

                PORTC = sandwichesMade & 0x3F;

                // if the button was pressed in time, we want to farm eggs instead of items
                bool getEggs = delay(500, 250, 2000, btn_make_sandwich);

                set_pin(led_input_type, getEggs);

                for (;;) {
                    set_leds(RX_LED);
                    send_current();
                    // make the sandwich
                    make_egg_sandwich();

                    // neutralize the controller
                    pause_automation();

                    beep(2);

                    // wait 30 minutes before doing another sandwich
                    lastSandwich = millis();
                    // reset the pickup timer
                    lastPickup = 0;

                    static ulong_t lastLedSwitch = 0;
                    uint8_t count = 0;
                    do {
                        if (button_held(btn_make_sandwich)) {
                            // if we ever press the button within those 30 minutes
                            // we stop the loop
                            beep(4);
                            cancel = true;

                            // if we press the button more than once
                            // we immediately exit the loop
                            if (count_button_presses(100, 900, btn_make_sandwich) > 1) {
                                break;
                            }
                        }

                        // switch the leds every 500ms
                        // so we know that we're waiting for the next item pickup
                        switch (count % 4) {
                            case 0:
                                set_leds(NO_LEDS);
                                send_current();
                            break;
                            case 1:
                                set_leds(TX_LED);
                                send_current();
                            break;
                            case 2:
                                set_leds(BOTH_LEDS);
                                send_current();
                            break;
                            case 3:
                                set_leds(RX_LED);
                                send_current();
                            break;
                        }

                        if (millis() - lastLedSwitch > 500) {
                            if (count >= 255)
                                count = 0;
                            else
                                count++;
                            lastLedSwitch = millis();
                        }

                        // if the last pickup was more than 160 seconds ago, try and pick up another item
                        if (millis() - lastPickup > 160000) {
                            // last pickup was more than 160 seconds ago
                            // so get an item from the basket

                            // emit a short beep so that we know we're trying to pick up an item
                            beep(1);

                            set_leds(BOTH_LEDS);
                            send_current();

                            // if its the first time we're doing this, we need to face the basket
                            // so we pass facingBasket as false
                            // and set it to true afterwards, so that the function knows that we're facing the basket
                            // and doesnt try to turn towards it
                            get_items_or_eggs_from_basket(facingBasket, getEggs);
                            // set facingBasket to true
                            facingBasket = true;

                            // update lastPickup
                            lastPickup = millis();

                            set_leds(NO_LEDS);
                            send_current();
                            // emit a short beep so we know we're done
                            beep(1);
                        }
                    } while (millis() - lastSandwich < 1800000); // 30 minutes

                    // if we've previously faced the basket, return view to the table
                    if (facingBasket) {
                        face_table();
                        facingBasket = false;
                    }

                    // and make another sandwich, aka restart the loop
                    beep(2);

                    // increase our binary counter, so we know how many times we've run this loop
                    // since we only have 6 leds, the highest number we can output is 2^6 = 64-1 = 63
                    if (sandwichesMade < 64) {
                        // output the number on the leds
                        PORTC = sandwichesMade & 0x3F; // 0x3F = 0011 1111 = 63 
                        sandwichesMade++;
                    }

                    if (cancel)
                        break;
                }
            }            
        }
    
        else if (button_held(btn_switch_to_virt)) {
            switch_controller(REAL_TO_VIRT);
        }
        else if (button_held(btn_switch_to_real)) {
            switch_controller(VIRT_TO_REAL);
        }
    }
}