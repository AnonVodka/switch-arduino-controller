#include <util/delay.h>

#include "automation-utils.h"
#include "user-io.h"


struct pin_t main_btn       = { PIN_MAIN_BUTTON, _PORTD, false };
struct pin_t alternate_btn  = { PIN_TEST_BUTTON, _PORTD, false };
struct pin_t catch_btn      = { PIN_TEST_BUTTON2, _PORTD, false };
struct pin_t dont_catch_btn = { PIN_TEST_BUTTON3, _PORTD, false };
struct pin_t switch_input   = { PIN_SWITCH_INPUT, _PORTD, false };
struct pin_t input_type     = { PIN_INPUT_TYPE, _PORTB, true };

int main() {

	init_automation();    

	init_pin(led_pin);
	init_pin(buzzer_pin);

	init_pin(main_btn);
	init_pin(alternate_btn);
	init_pin(catch_btn);
	init_pin(dont_catch_btn);
	init_pin(switch_input);
	init_pin(input_type);

	// /* Initial beep to confirm that the buzzer works */
	beep(1);
    
	/* Wait for the user to press the button (should be on the Switch main menu) */
	count_button_presses(100, 100, main_btn);
	beep(2);

    bool is_virt = false;
    bool in_raid = false;

    for (;;) { 

        pause_automation();

        if (button_held(alternate_btn) && button_held(dont_catch_btn)) {
            send_update(BT_A, DP_NEUTRAL, S_NEUTRAL, S_NEUTRAL);
        }

        if (button_held(switch_input)) {
            if (!button_held(alternate_btn)) {
                if (is_virt) {
                    switch_controller(VIRT_TO_REAL);                
                    is_virt = false;
                }
                else {
                    switch_controller(REAL_TO_VIRT);
                    is_virt = true;
                }
                set_pin(input_type, is_virt);
            }
            else {
                // SEND_BUTTON_SEQUENCE(
                //     /* Fix camera in case its not straight */
                //     {BT_L, DP_NEUTRAL, SEQ_HOLD, 10}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 1}
                // );

                /* Walk backwards */
                for (uint8_t i = 0 ; i < 25 ; i += 1) {
                    send_update(BT_NONE, DP_NEUTRAL, S_BOTTOM, S_NEUTRAL);
                }

                pause_automation();

                SEND_BUTTON_SEQUENCE(
                    /* Fix camera */
                    {BT_L, DP_NEUTRAL, SEQ_HOLD, 10}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 1}
                );
            }
        }

        if (button_held(main_btn)) {
            if (!button_held(alternate_btn) /* && !in_raid */) {
                /* use wishing piece */
                SEND_BUTTON_SEQUENCE(
                    /* Empty dialog */
                    {BT_A, DP_NEUTRAL, SEQ_HOLD, 8}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 1},
                    /* Throw wishing piece */
                    {BT_A, DP_NEUTRAL, SEQ_HOLD, 8}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 15},
                    /* Accept */
                    {BT_A, DP_NEUTRAL, SEQ_HOLD, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 35},
                ); 
                // beep(3);
                SEND_BUTTON_SEQUENCE(
                    /* Save dialog */
                    {BT_A, DP_NEUTRAL, SEQ_HOLD, 15}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 1},
                    /* Accept */
                    {BT_A, DP_NEUTRAL, SEQ_HOLD, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 25},

                    /* Wishing piece thrown */
                    {BT_A, DP_NEUTRAL, SEQ_HOLD, 20}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 15},
                    /* Close dialog */
                    {BT_A, DP_NEUTRAL, SEQ_HOLD, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 15},
                )
                beep(1);
                _delay_ms(100);
                // /* enter raid */
                SEND_BUTTON_SEQUENCE(
                    /* Interact with den and wait for menu to open */
                    {BT_A, DP_NEUTRAL, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 10}, 
                    /* Scroll down to "Solo" */
                    {BT_NONE, DP_DOWN, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 1}, 
                    /* Hold a to fasten solo dialog */
                    {BT_A, DP_NEUTRAL, SEQ_HOLD, 10}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 10}, 
                    /* Accept dialog and wait 40 seconds for the raid to load*/
                    {BT_A, DP_NEUTRAL, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 15},  
                )
                _delay_ms(40000);
                beep(1);
                in_raid = true;
            } 
            else {
                uint8_t attacks = count_button_presses(500, 500, main_btn);

                for (uint8_t i = 0 ; i < attacks ; i += 1) {
                    beep(1);
                    _delay_ms(100);
                }

                for (uint8_t i = 0 ; i < attacks ; i += 1) {
                    /* attack sequence */
                    SEND_BUTTON_SEQUENCE(
                        /* Open the attack field and wait for it to load */
                        {BT_A, DP_NEUTRAL, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 8}, 
                        /* Use first attack on the list */
                        {BT_A, DP_NEUTRAL, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 15}, 
                        /* Select dynamaxed pokemon and wait roughly 60 seconds for all attacks to finish*/
                        {BT_A, DP_NEUTRAL, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 10}, 
                    )
                    if (i != attacks-1)
                        // only delay when we're not on the last attack
                        // so that we dont have to wait 70 seconds incase the pokemon dies lol
                        _delay_ms(70000);
                    beep(1);
                }
            }
        }
        if (button_held(catch_btn) /* && in_raid */) {
            bool new_pokemon = button_held(alternate_btn);
            /* fight over */
            SEND_BUTTON_SEQUENCE(
                /* Select "catch pokemon" and wait for the pokeball menu to open*/
                {BT_A, DP_NEUTRAL, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 15}, 
                /* Select first pokeball and wait roughly 35 seconds for the animation to finish*/
                {BT_A, DP_NEUTRAL, SEQ_MASH, 2}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 875}, 
                /* Press a to show the pokemon */
                {BT_A, DP_NEUTRAL, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 250},
            )
            beep(2); 
            if (new_pokemon) {
                SEND_BUTTON_SEQUENCE(
                    /* Press a to show the pokemon details and wait 150ms for them to load */
                    {BT_A, DP_NEUTRAL, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 5},
                    /* Press a to close the pokemon details and wait 150ms for them to close */
                    {BT_A, DP_NEUTRAL, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 5},
                )
                beep(1);
            }
            SEND_BUTTON_SEQUENCE(
                /* Hold b to skip nickname and fasten dialog */
                {BT_B, DP_NEUTRAL, SEQ_HOLD, 10}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 1},
                /* Hold b to send to box and fasten dialog */
                {BT_B, DP_NEUTRAL, SEQ_HOLD, 10}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 1},
                /* Press a to end raid and wait roughly 10 seconds to restart loop*/
                {BT_A, DP_NEUTRAL, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 250},
            )
            beep(2);
            in_raid = false;
        }
        if (button_held(dont_catch_btn) /* && in_raid */) {
            SEND_BUTTON_SEQUENCE(
                /* Go down to "dont catch" */
                {BT_NONE, DP_DOWN, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 1}, 
                /* Press a to not catch pokemon and wait for rewards screen to load*/
                {BT_A, DP_NEUTRAL, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 50},
                /* Press a and wait for the game to load */
                {BT_A, DP_NEUTRAL, SEQ_MASH, 1}, {BT_NONE, DP_NEUTRAL, SEQ_HOLD, 250}, 
            )
            beep(2); 
            in_raid = false;
        }
    }
}