/*
 * Pokémon Sword/Shield automation
 */

#include <util/delay.h>

#include "automation-utils.h"
#include "user-io.h"

/* Static functions */
static void release_full_boxes(void);
static void position_box_cursor_topleft(void);
static bool for_each_box_pos(bool top_left_start, bool (*callback)(void));
static bool release_from_box(void);

struct pin_t main_btn   = { PIN_MAIN_BUTTON, _PORTD, false };

int main(void)
{
	init_automation();
	init_pin(main_btn);
	init_pin(led_pin);
	init_pin(buzzer_pin);

	/* Initial beep to confirm that the buzzer works */
	beep(1);

	for (;;) {
		/* Set the LEDs, and make sure automation is paused while in the
		   menu */
		set_leds(BOTH_LEDS);
		pause_automation();
		
		beep(1);
		/* Feature selection menu */
		uint8_t count = count_button_presses(100, 900, main_btn);

		switch (count ){
			case 1:
				/* Release all Pokémon in the current box */
				release_full_boxes();
				break;
			case 2:
				switch_controller(VIRT_TO_REAL);
				break;
			case 3:
				switch_controller(REAL_TO_VIRT);
				break;
		}
	}
}

/*
 * From the Box menu, releases all Pokémon in the current box, then move
 * to the next Box. User confirmation is asked for each Box. The Boxes must
 * be completely full.
 */
void release_full_boxes(void)
{
	//position_box_cursor_topleft();

	bool cursor_topleft = true;

	for (;;) {
		/* Wait for user confirmation */
		beep(1);
		if (count_button_presses(500, 500, main_btn) > 1) {
			/* User cancelled, we are done */
			return;
		}

		/* Release the Box content */
		for_each_box_pos(cursor_topleft, &release_from_box);

		/* The cursor position was toggled by the operation */
		cursor_topleft ^= true;

		/* Move to the next Box */
		SEND_BUTTON_SEQUENCE(
			{ BT_R,	DP_NEUTRAL,	SEQ_MASH,	1 },	/* Next Box */
		);
	}
}

/*
 * Position the cursor to the top left Pokémon in the Box menu.
 */
void position_box_cursor_topleft(void)
{
	/* We hold the D-pad to put the cursor in a known position (mashing it does
	   not work since it makes the cursor roll around the edges of the screen).
	   We need to avoir getting the cursor on the Box title since the D-pad
	   will start changing the selected Box. The screen layout also tend to
	   make the cursor stuck in random positions if diagonal directions are
	   used.*/

	SEND_BUTTON_SEQUENCE(
		{ BT_NONE,	DP_DOWN,	SEQ_HOLD,	25 },	/* Bottom row */
		{ BT_NONE,	DP_LEFT,	SEQ_HOLD,	25 },	/* Last Pokémon */
		{ BT_NONE,	DP_UP,		SEQ_MASH,	5  },	/* First team Pokémon */
		{ BT_NONE,	DP_RIGHT,	SEQ_MASH,	1  },	/* Top/Left Box Pokémon */
	);
}


/*
 * Calls a callback after positioning the cursor on each Pokémon in the Box.
 * The starting position can either be the top left Pokémon or the bottom
 * right. The ending cursor position will be the reverse of the starting
 * cursor position.
 * Stops the process and return true if the callback returns true; else
 * returns false.
 */
bool for_each_box_pos(bool top_left_start, bool (*callback)(void))
{
	/* Do we go left on even rows (row 0, row 2, etc)? */
	uint8_t left_on_even = (top_left_start ? 0 : 1);

	/* Which direction to use to move between rows? */
	enum d_pad_state change_row_dir = (top_left_start ? DP_DOWN : DP_UP);

	for (uint8_t row = 0 ; row < 5 ; row += 1) {
		for (uint8_t col = 0 ; col < 5 ; col += 1) {
			enum d_pad_state move_dir;

			//SINGLE_STEP(main_btn);

			pause_automation(); _delay_ms(100);

			if (callback()) {
				return true;
			}

			pause_automation(); _delay_ms(100);

			//SINGLE_STEP(main_btn);

			if ((row % 2) == left_on_even) {
				move_dir = DP_RIGHT;
			} else {
				move_dir = DP_LEFT;
			}

			pause_automation(); _delay_ms(100);

			//SINGLE_STEP(main_btn);

			SEND_BUTTON_SEQUENCE(
				{ BT_NONE,	move_dir,	SEQ_MASH,	1 },
				{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 },
			);
		}
		
		pause_automation(); _delay_ms(100);

		//SINGLE_STEP(main_btn);

		if (callback()) {
			return true;
		}

		pause_automation(); _delay_ms(100);

		//SINGLE_STEP(main_btn);

		if (row < 4) {
			SEND_BUTTON_SEQUENCE(
				{ BT_NONE,	change_row_dir,	SEQ_MASH,	1 },
				{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1 },
			);
		}
	}

	return false;
}


/*
 * Release from the Box the Pokémon on which the cursor is on.
 * Returns false so for_each_box_pos continues execution.
 */
bool release_from_box(void)
{
	SEND_BUTTON_SEQUENCE(
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	8  },	/* Open menu */
		{ BT_NONE,	DP_UP,		SEQ_MASH,	2  },	/* Go to option */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	20  },	/* Select option */
		{ BT_NONE,	DP_UP,		SEQ_MASH,	1  },	/* Go to Yes */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	25  },	/* Validate dialog 1 */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Release 1 */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	10  },	/* Validate dialog 2 */
		{ BT_NONE,	DP_UP,		SEQ_MASH,	1  },	/* Go to Yes */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	25  },	/* Validate dialog 1 */
		{ BT_NONE,	DP_NEUTRAL,	SEQ_HOLD,	1  },	/* Release 1 */
		{ BT_A,		DP_NEUTRAL,	SEQ_HOLD,	10  },	/* Validate dialog 2 */
	);

	return false;
}