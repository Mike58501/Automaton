#include <Automaton.h>

int led1Pin = 5;
int led2Pin = 6;

Atm_fade led1, led2;

void setup() {
  led1.begin( led1Pin ).blink( 200 ).fade( 5 );
  led2.begin( led2Pin ).blink( 500 ).fade( 10 );
  led1.trigger( led1.EVT_BLINK );
  led2.trigger( led2.EVT_BLINK );
}

void loop() {
  led1.cycle();
  led2.cycle();
}