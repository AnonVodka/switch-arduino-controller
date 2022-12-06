#ifndef UTILS_H
#define UTILS_H
typedef unsigned long ulong_t;
volatile ulong_t timer1_millis;
//NOTE: A unsigned long holds values from 0 to 4,294,967,295 (2^32 - 1). It will roll over to 0 after reaching its maximum value.

ISR(TIMER1_COMPA_vect);
void init_millis(ulong_t f_cpu);
ulong_t millis (void);

uint32_t pseudo_random(uint32_t min, uint32_t max);
#endif