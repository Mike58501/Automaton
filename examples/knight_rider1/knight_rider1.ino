#include <Automaton.h>

Atm_led led[6];
Atm_timer timer;
Atm_step step;
Appliance app;

// Timer drives step sequencer in sweep mode, step sequencer blinks leds.

short event = Atm_led::EVT_BLINK;
static short pin_list[] = { 4, 5, 6, 7, 8, 9 };
int blink_time = 70;
int interval_time = 50;

void setup() {
  // Initialize the step sequencer
  app.component( 
    step.begin()
      .trigger( Atm_step::EVT_SWEEP ) 
  );
  
  // Add the timer
  app.component( 
    timer.begin( interval_time )
      .onTimer( step, Atm_step::EVT_STEP )
      .repeat( ATM_COUNTER_OFF ) 
  );
  
  // Add the leds and link them to the step sequencer
  for ( short i = 0; i <= 5; i++ ) {
    app.component( 
      led[i].begin( pin_list[i] )
        .blink( blink_time, 1, 1 ) 
    ); 
    step.onStep( i, led[i], event );
  }
  
  // Move last led from step 5 to step 7 to make sweep work properly!
  step.onStep( 5 ).onStep( 9, led[5], event );
  timer.trigger( timer.EVT_START );
}

void loop() {
  app.run();
}
