
#ifndef Atm_led_h
#define Atm_led_h

#include <Automaton.h>

class Atm_led : public Machine {

  public:
	Atm_led( void ) : Machine() { class_label = "LED"; };

	enum { IDLE, ON, START, BLINK_OFF, DONE }; 
	enum { EVT_ON_TIMER, EVT_OFF_TIMER, EVT_COUNTER, EVT_ON, EVT_OFF, EVT_BLINK, EVT_TOGGLE, EVT_TOGGLE_BLINK, ELSE }; 
	enum { ACT_INIT, ACT_ON, ACT_OFF, ACT_CHAIN }; 
		
	Atm_led & begin( int attached_pin );
	Atm_led & blink( uint32_t duration ); 
	Atm_led & pause( uint32_t duration ); 
	Atm_led & fade( int fade ); 
	Atm_led & repeat( int repeat ); 
    Atm_led & trace( Stream * stream );
    Atm_led & chain( Machine * n = 0, Machine * p = 0, uint8_t event = EVT_BLINK );
    
  private:  
	short pin;
	int repeat_count;
	atm_timer_millis on_timer, off_timer;
	atm_counter counter;
    Machine * chain_next;
    Machine * chain_previous;
    uint8_t chain_event;

	int event( int id );
	void action( int id );
};


class Att_led : public TinyMachine {

  public:
	Att_led( void ) : TinyMachine() { };

	enum { IDLE, ON, START, BLINK_OFF, DONE }; 
	enum { EVT_ON_TIMER, EVT_OFF_TIMER, EVT_COUNTER, EVT_ON, EVT_OFF, EVT_BLINK, ELSE }; 
	enum { ACT_INIT, ACT_ON, ACT_OFF, ACT_CHAIN }; 
		
	Att_led & begin( int attached_pin );
	Att_led & blink( uint32_t duration ); 
	Att_led & pause( uint32_t duration ); 
	Att_led & fade( int fade ); 
	Att_led & repeat( int repeat ); 
    Att_led & chain( TinyMachine * n = 0, TinyMachine * p = 0, uint8_t event = EVT_BLINK );
  
  private:
	short pin;
	int repeat_count;
	atm_timer_millis on_timer, off_timer;
	atm_counter counter;
    TinyMachine * chain_next;
    TinyMachine * chain_previous;
    uint8_t chain_event;
	int event( int id );
	void action( int id );
};

#endif
