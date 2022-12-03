#include <util/delay.h>

#include "automation-utils.h"
#include "user-io.h"

struct pin_t refresh_raids_btn  = { PIN_MAIN_BUTTON, _PORTD, false };       // black wire
struct pin_t switch_input_btn   = { PIN_TEST_BUTTON, _PORTD, false };       // white wire
struct pin_t a_button           = { PIN_TEST_BUTTON2, _PORTD, false };      // gray wire
struct pin_t input_type         = { PIN_INPUT_TYPE, _PORTB, true };         // led

int main() {

	init_automation();    

	init_pin(led_pin);
	init_pin(buzzer_pin);

	init_pin(refresh_raids_btn);
	init_pin(switch_input_btn);
	init_pin(a_button);
	init_pin(input_type);

	// /* Initial beep to confirm that the buzzer works */
	beep(1);
    
	/* Wait for the user to press the button (should be on the Switch main menu) */
	count_button_presses(100, 100, refresh_raids_btn);
	beep(2);

    bool is_virt = false;
    bool in_raid = false;

    for (;;) { 

        pause_automation();

        if (button_held(&refresh_raids_btn)) {
            beep(1);
            change_clock_day(false, 1);
            beep(1);
        }
        if (button_held(&switch_input_btn)) {
             beep(2);
             if (is_virt) {
                switch_controller(VIRT_TO_REAL);
                is_virt = false;
             }
             else {
                switch_controller(REAL_TO_VIRT);
                is_virt = true;
             }
             set_pin(&input_type, is_virt);
             beep(2);
        }
        if (button_held(&a_button)) {
            beep(3);
            SEND_BUTTON_SEQUENCE(
			    { BT_A,			DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Register as controller 1 */
			    { BT_NONE,		DP_NEUTRAL,	SEQ_HOLD,	15 },	/* Wait for registration */
		    );
            beep(3);
        }
    }
}