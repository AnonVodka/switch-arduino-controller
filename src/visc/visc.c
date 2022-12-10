#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "automation-utils.h"
#include "user-io.h"
#include "utils.h"

pin_t btn_refresh_dais      = { PIN_MAIN_BUTTON,    _PORTD, false   };      // black wire
pin_t btn_make_sandwich     = { PIN_TEST_BUTTON,    _PORTD, false   };      // white wire
pin_t btn_buttons           = { PIN_TEST_BUTTON2,   _PORTD, false   };      // gray wire
pin_t btn_switch_to_virt    = { PIN_TEST_BUTTON3,   _PORTD, false   };      // purple wire
pin_t btn_switch_to_real    = { PIN_SWITCH_INPUT,   _PORTD, false   };      // blue wire
pin_t led_input_type        = { PIN_INPUT_TYPE,     _PORTB, true    };      // led

ulong_t lastSandwich = 0; 
ulong_t lastPickup = 0;

void change_wallpaper() {
    
    // press a and wait 5 cycles
    SEND_BUTTON_SEQUENCE(
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1   },	
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	5   },
    );

    // go down once and wait 5 cycles and press a
    SEND_BUTTON_SEQUENCE(
        { BT_NONE,	DP_BOTTOM,	SEQ_MASH,	1   },	
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	2   },
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1   },	
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	10   },
    );

    // go up 6 times and wait 2 cycles in between each press
    // it just skips 1 button press alltogether, no matter the delay lol
    SEND_BUTTON_SEQUENCE(
        { BT_NONE, DP_TOP, SEQ_HOLD, 1 },
        { BT_NONE, DP_NEUTRAL, SEQ_HOLD, 10},
        { BT_NONE, DP_TOP, SEQ_HOLD, 1 },
        { BT_NONE, DP_NEUTRAL, SEQ_HOLD, 10},
        { BT_NONE, DP_TOP, SEQ_HOLD, 1 },
        { BT_NONE, DP_NEUTRAL, SEQ_HOLD, 10},
        { BT_NONE, DP_TOP, SEQ_HOLD, 1 },
        { BT_NONE, DP_NEUTRAL, SEQ_HOLD, 10},
        { BT_NONE, DP_TOP, SEQ_HOLD, 1 },
        { BT_NONE, DP_NEUTRAL, SEQ_HOLD, 10},
        { BT_NONE, DP_TOP, SEQ_HOLD, 1 },
        { BT_NONE, DP_NEUTRAL, SEQ_HOLD, 10},
    )

    // press a to select wallpaper wait 5 cycles and press r once to go to the right
    SEND_BUTTON_SEQUENCE(
        { BT_A,		DP_NEUTRAL,	SEQ_MASH,	1   },	
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	10   },
        { BT_R,		DP_NEUTRAL,	SEQ_MASH,	1   },	
        { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	10   },
    );

}

void make_egg_sandwich() {
    /*
    interact with table
        - a, 50 delay
    choose "make a sandwhich"
        - a, 50 delay
    choose "great peanut butter sandwhich"
        - down, down, right, a, 15, a, 75, stick up 1.5s, hold a, stick down 1.5s, repeat 3 times, stay dont, a, 50, a, 250, a, repeat
    */

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
    SEND_BUTTON_SEQUENCE(
        { BT_NONE,  DP_BOTTOM,  SEQ_MASH,   5   }, // got down
        { BT_NONE,  DP_NEUTRAL, SEQ_HOLD,   3   },
        { BT_NONE,  DP_RIGHT,   SEQ_MASH,   1   }, // go right
        { BT_NONE,  DP_NEUTRAL, SEQ_HOLD,   2   },
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

    if (!get_eggs) {
        // we dont want to get any eggs from the basket
        // so we can press a once and then spam b
        // since there wont be a popup asking us if we want to pick the item up
        // this is used for the egg item duplication glitch


        // press a once to interact with the basket
        SEND_BUTTON_SEQUENCE(
            { BT_A,		DP_NEUTRAL,	SEQ_HOLD,	3 },	// fast forward dialog box
            { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
        );

        // then spam b for 50 cycles, aka 2 seconds
        for (uint8_t i = 0; i < 50; i++) {
            SEND_BUTTON_SEQUENCE(
                { BT_B,		DP_NEUTRAL,	SEQ_HOLD,	3 },	// fast forward dialog box
                { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 }
            );
        } 
    }
    else {
        // we're trying to pick up eggs from the basket
        // we we press a once and then press it like 5 times more

        beep(1);

        SEND_BUTTON_SEQUENCE(
            { BT_A,		DP_NEUTRAL,	SEQ_HOLD,	3 }, // "You peeked inside the basket" dialog
        );

        SINGLE_STEP(btn_buttons);

        SEND_BUTTON_SEQUENCE(
            { BT_B,		DP_NEUTRAL,	SEQ_HOLD,	3 }, // Close dialog
        );

        SINGLE_STEP(btn_buttons);

        SEND_BUTTON_SEQUENCE(
            { BT_A,		DP_NEUTRAL,	SEQ_HOLD,	3 }, // "There is a egg inside!" Dialog
        );

        SINGLE_STEP(btn_buttons);

        for (uint8_t i = 0; i < 5; i++) {
            SEND_BUTTON_SEQUENCE(
                { BT_A,		DP_NEUTRAL,	SEQ_HOLD,	3 }, // Close Dialog and open "Yes/No" Dialog
            );

            SINGLE_STEP(btn_buttons);

            SEND_BUTTON_SEQUENCE(
                { BT_A,		DP_NEUTRAL,	SEQ_HOLD,	3 }, // Choose "Yes" and open "You took the Egg!" Dialog
            );

            SINGLE_STEP(btn_buttons);

            SEND_BUTTON_SEQUENCE(
                { BT_A,		DP_NEUTRAL,	SEQ_HOLD,	3 }, // Close Dialog
            );

            SINGLE_STEP(btn_buttons);

            SEND_BUTTON_SEQUENCE(
                { BT_A,		DP_NEUTRAL,	SEQ_HOLD,	3 }, // "There is something else" Dialog
            );
        }
    }
}



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

	init_pin(btn_refresh_dais);
	init_pin(btn_switch_to_virt);
	init_pin(btn_buttons);
	init_pin(btn_make_sandwich);
    init_pin(btn_switch_to_real);
	init_pin(led_input_type);

	/* Initial beep to confirm that the buzzer works */
	beep(1);
    
	/* Wait for the user to press the button */
	count_button_presses(100, 100, btn_refresh_dais);
	beep(2);

    for (;;) { 
		set_leds(BOTH_LEDS);
        pause_automation();

        if (button_held(btn_refresh_dais)) {
            beep(1);
            change_clock_day(false, 1);
            beep(1);
        }
        else if (button_held(btn_buttons)) {
            beep(3);
            uint8_t presses = count_button_presses(100, 100, btn_buttons);
            beep(presses);
            if (presses == 1) {
                SEND_BUTTON_SEQUENCE(
                    { BT_A,		DP_NEUTRAL,	SEQ_HOLD,	1 },
                    { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 },
                );
            }
            else if (presses == 2) {
                SEND_BUTTON_SEQUENCE(
                    { BT_B,		DP_NEUTRAL,	SEQ_HOLD,	1  },
                    { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 },
                );
            }
            else if (presses == 3) {
                SEND_BUTTON_SEQUENCE(
                    { BT_HOME,	DP_NEUTRAL,	SEQ_HOLD,	1 },
                    { BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 },
                );
            }
            else if (presses == 4) {
                get_items_or_eggs_from_basket(true, true);
            }
            
            else if (presses == 5) {
                face_basket();
            }
            
            else if (presses == 6) {
                //for (uint8_t i = 0; i < 32; i++) {
                    change_wallpaper();
                //}
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
                uint8_t binaryCounter = 0;
                PORTC = binaryCounter & 0x3F;
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
                            break;
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
                            get_items_or_eggs_from_basket(facingBasket, false);
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
                    if (binaryCounter < 64) {
                        // output the number on the leds
                        PORTC = binaryCounter & 0x3F; // 0x3F = 0011 1111 = 63 
                        binaryCounter++;
                    }

                    if (cancel)
                        break;
                }
            }            
        }
    
        else if (button_held(btn_switch_to_virt)) {
            switch_controller(REAL_TO_VIRT);
            set_pin(led_input_type, true);
        }
        else if (button_held(btn_switch_to_real)) {
            switch_controller(VIRT_TO_REAL);
            set_pin(led_input_type, false);
        }
    }
}