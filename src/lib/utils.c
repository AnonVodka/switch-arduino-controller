#include <avr/io.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

#include "utils.h"

ISR(TIMER1_COMPA_vect)
{
  timer1_millis++;
}

void init_millis(ulong_t f_cpu)
{
  ulong_t ctc_match_overflow;

  ctc_match_overflow = ((f_cpu / 1000) / 8); //when timer1 is this value, 1ms has passed

  // (Set timer to clear when matching ctc_match_overflow) | (Set clock divisor to 8)
  TCCR1B |= (1 << WGM12) | (1 << CS11);

  // high byte first, then low byte
  OCR1AH = (ctc_match_overflow >> 8);
  OCR1AL = ctc_match_overflow;

  // Enable the compare match interrupt
  TIMSK1 |= (1 << OCIE1A);

  //REMEMBER TO ENABLE GLOBAL INTERRUPTS AFTER THIS WITH sei(); !!!
}

ulong_t millis (void)
{
  ulong_t millis_return;

  // Ensure this cannot be disrupted
  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    millis_return = timer1_millis;
  }
  return millis_return;
}

// Seed value for the pseudo-random number generator
static uint32_t seed = 1;

// Function to generate a pseudo-random number using bitwise operations
uint32_t pseudo_random(uint32_t min, uint32_t max) {
  // Use a linear congruential generator to generate the next pseudo-random number
  seed = (1103515245 * seed + 12345) & 0x7fffffff;

  // Scale the generated number to the specified range
  uint32_t range = max - min + 1;
  uint32_t r = min + (seed % range);

  return r;
}