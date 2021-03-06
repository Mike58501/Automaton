#include <Automaton.h>
#include "Atm_sweep.h"

Atm_sweep sweep;
Atm_button button;
Appliance app;

void setup() {
  // Create a sweep machine and add it to the app
  app.component( 
    sweep.begin( 4, 5, 6, 7, 8, 9 )
      .speed( 50 ) 
  );
  // Create a button machine, link it to the sweep machine and add it to the app
  app.component( 
    button.begin( 2 )
      .onPress( sweep, sweep.EVT_TOGGLE ) 
  );
}

void loop() {
  app.run();
}
