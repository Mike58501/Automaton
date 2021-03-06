/*
  Automaton.cpp - Reactive State Machine Framework for Arduino.
  Published under the MIT License (MIT), Copyright (c) 2015-2016, J.P. van der Landen
*/

#include "Automaton.h"

/*
 * The atm_connector class facilitates creating push and pull connections between
 * State Machines.
 *
 *********************************************************************************************
 *
 * push( v, up, noCallback ) - Calls a machine's trigger method or a callback
 *
 * Will return false if a callback is configured while the noCallback arg was specified
 */

bool atm_connector::push( int v /* = 0 */, int up /* = 0 */, bool noCallback /* = false */ ) {
  switch ( mode_flags & B00000111 ) {
    case MODE_PUSHCB:
      if ( noCallback ) {
        return false;
      } else {
        ( *push_callback )( callback_idx, v, up );
      }
      return true;
    case MODE_MACHINE:
      machine->trigger( event );
      return true;
  }
  return true;
}

/*
 * pull( v, up, def_value ) - Calls a machine's state method or a callback
 *
 */

int atm_connector::pull( int v /* = 0 */, int up /* = 0 */, bool def_value /* = false */ ) {
  switch ( mode_flags & B00000111 ) {
    case MODE_PULLCB:
      return ( *pull_callback )( callback_idx );
    case MODE_MACHINE:
      return machine->state();
  }
  return def_value;
}

/*
 * logOp() Returns the logical operator part of the mode_flags byte
 *
 */

int8_t atm_connector::logOp( void ) {
  return ( mode_flags & B00011000 ) >> 3;
}

/*
 * logOp() Returns the relational operator part of the mode_flags byte
 *
 */

int8_t atm_connector::relOp( void ) {
  return ( mode_flags & B11100000 ) >> 5;
}

/*
 * set( callback, idx, logOp, relOp ) - Configures a connector object as a push callback
 *
 */

void atm_connector::set( atm_cb_push_t callback, int idx, int8_t logOp /* = 0 */, int8_t relOp /* = 0 */ ) {
  mode_flags = MODE_PUSHCB | ( logOp << 3 ) | ( relOp << 5 );
  push_callback = callback;
  callback_idx = idx;
}

/*
 * set( callback, idx, logOp, relOp ) - Configures a connector object as a pull callback
 *
 */

void atm_connector::set( atm_cb_pull_t callback, int idx, int8_t logOp /* = 0 */, int8_t relOp /* = 0 */ ) {
  mode_flags = MODE_PULLCB | ( logOp << 3 ) | ( relOp << 5 );
  pull_callback = callback;
  callback_idx = idx;
}

/*
 * set( callback, idx, logOp, relOp ) - Configures a connector object as a machine connector (calls trigger)
 *
 */

void atm_connector::set( Machine* m, int evt, int8_t logOp /* = 0 */, int8_t relOp /* = 0 */ ) {
  mode_flags = MODE_MACHINE | ( logOp << 3 ) | ( relOp << 5 );
  machine = m;
  event = evt;
}

/*
 * mode() - Returns the mode part of the mode_flags byte
 *
 */

int8_t atm_connector::mode( void ) {
  return mode_flags & B00000111;
}

/*
 * atm_timer_millis::set( v ) - Sets a millis timer to 'v'
 *
 */

void atm_timer_millis::set( uint32_t v ) {
  value = v;
}

/*
 * atm_timer_millis::expired( this ) - Checks a millis timer for expiry (== 0)
 *
 */

int atm_timer_millis::expired( Machine* machine ) {
  return value == ATM_TIMER_OFF ? 0 : millis() - machine->state_millis >= value;
}

/*
 * atm_counter::set( v ) - Sets a countdown counter to 'v'
 *
 */

void atm_counter::set( uint16_t v ) {
  value = v;
}

/*
 * atm_counter::decrement() - Decrements a countdown counter
 *
 */

uint16_t atm_counter::decrement( void ) {
  return value > 0 && value != ATM_COUNTER_OFF ? --value : 0;
}

/*
 * atm_counter::expired() - Checks a countdown counter for expiry (== 0)
 *
 */

uint8_t atm_counter::expired() {
  return value == ATM_COUNTER_OFF ? 0 : ( value > 0 ? 0 : 1 );
}

/* The Machine class is a base class for creating and running State Machines
 *
 *********************************************************************************************
 *
 * Machine::state( void ) - Retrieves the current state for the machine
 *
 * (may be overridden by a subclass in which case it may return something else, like a value )
 */

int Machine::state() {
  return current;
}

/*
 * Machine::state( state ) - Sets the next state for the machine
 *
 */

Machine& Machine::state( int state ) {
  next = state;
  last_trigger = -1;
  flags &= ~ATM_SLEEP_FLAG;
  return *this;
}

/*
 * Machine::trigger( evt ) - Triggers an event for the machine
 *
 * The machine is cycled for maximum of 8 times until it is actively listening for the event
 * Then the event is triggered followed by two more cycles to process the event and the
 * following state change.
 *
 */

Machine& Machine::trigger( int evt /* = 0 */ ) {
  int new_state;
  int max_cycle = 8;
  do {
    flags &= ~ATM_SLEEP_FLAG;
    cycle();
    new_state = read_state( state_table + ( current * state_width ) + evt + ATM_ON_EXIT + 1 );
  } while ( --max_cycle && ( new_state == -1 || next_trigger != -1 ) );
  if ( new_state > -1 ) {
    next_trigger = evt;
    flags &= ~ATM_SLEEP_FLAG;
    cycle();  // Pick up the trigger
    flags &= ~ATM_SLEEP_FLAG;
    cycle();  // Process the state change
  }
  return *this;
}

/*
 * Machine::setTrace( stream, callback, symbols ) - Sets up state tracing for the machine
 *
 * Connects a stream object, a callback (atm_serial_debug) and a symbol table (string) to the object
 *
 */

Machine& Machine::setTrace( Stream* stream, swcb_sym_t callback, const char symbols[] ) {
  callback_trace = callback;
  stream_trace = stream;
  this->symbols = symbols;
  return *this;
}

/*
 * Machine::sleep( v ) - Sets or returns the current sleep flag setting
 *
 */

uint8_t Machine::sleep( int8_t v /* = 1 */ ) {
  if ( v > -1 ) flags = v ? flags | ATM_SLEEP_FLAG : flags & ~ATM_SLEEP_FLAG;
  return ( flags & ATM_SLEEP_FLAG ) > 0;
}

/*
 * Machine::begin( state_table, width ) - Initializes the state table and sets the sleep flag
 *
 */

Machine& Machine::begin( const state_t* tbl, int width ) {
  state_table = tbl;
  state_width = ATM_ON_EXIT + width + 2;
  flags &= ~ATM_SLEEP_FLAG;
  return *this;
}

/*
 * Machine::mapSymbol( id, map ) - Maps a number ( event/state ) to a symbol
 *
 * 0        Machine class name (e.g. LED)
 * 1..ELSE  Event name (e.g. EVT_TIMER)
 * ELSE..   State name (e.g. IDLE)
 *
 */

const char* Machine::mapSymbol( int id, const char map[] ) {
  int cnt = 0;
  int i = 0;
  if ( id == -1 ) return "*NONE*";
  if ( id == 0 ) return map;
  while ( 1 ) {
    if ( map[i] == '\0' && ++cnt == id ) {
      i++;
      break;
    }
    i++;
  }
  return &map[i];
}

/*
 * Machine::cycle( time ) - Executes one cycle of a State Machine
 *
 * For every state change:
 * - Calls the ON_SWITCH action
 * - Calls the state trace function (if connected)
 * - Calls the previous state's ON_EXIT action
 * - Changes the active state (current) to the new
 * - Calls the new state's ON_ENTER action
 *
 * For every 'normal' cycle:
 * - Executes the ON_LOOP action
 * - Scans the event columns in the current table and calls active events
 *
 * If the 'time' argument is given, loops until that time has passed
 * otherwise executes only one cycle of the machine
 */

Machine& Machine::cycle( uint32_t time /* = 0 */ ) {
  uint32_t cycle_start = millis();
  do {
    if ( ( flags & ( ATM_SLEEP_FLAG | ATM_CYCLE_FLAG ) ) == 0 ) {
      cycles++;
      flags |= ATM_CYCLE_FLAG;
      if ( next != -1 ) {
        action( ATM_ON_SWITCH );
        if ( callback_trace ) {
          callback_trace( stream_trace, *this, symbols, mapSymbol( current == -1 ? current : current + state_width - ATM_ON_EXIT, symbols ),
                          mapSymbol( next == -1 ? next : next + state_width - ATM_ON_EXIT, symbols ), mapSymbol( last_trigger + 1, symbols ),
                          millis() - state_millis, cycles );
        }
        if ( current > -1 ) action( read_state( state_table + ( current * state_width ) + ATM_ON_EXIT ) );
        current = next;
        next = -1;
        state_millis = millis();
        action( read_state( state_table + ( current * state_width ) + ATM_ON_ENTER ) );
        if ( read_state( state_table + ( current * state_width ) + ATM_ON_LOOP ) == ATM_SLEEP ) {
          flags |= ATM_SLEEP_FLAG;
        } else {
          flags &= ~ATM_SLEEP_FLAG;
        }
        cycles = 0;
      }
      state_t i = read_state( state_table + ( current * state_width ) + ATM_ON_LOOP );
      if ( i != -1 ) {
        action( i );
      }
      for ( i = ATM_ON_EXIT + 1; i < state_width; i++ ) {
        state_t next_state = read_state( state_table + ( current * state_width ) + i );
        if ( ( next_state != -1 ) && ( i == state_width - 1 || event( i - ATM_ON_EXIT - 1 ) || next_trigger == i - ATM_ON_EXIT - 1 ) ) {
          state( next_state );
          last_trigger = i - ATM_ON_EXIT - 1;
          next_trigger = -1;
          break;
        }
      }
      flags &= ~ATM_CYCLE_FLAG;
    }
  } while ( millis() - cycle_start < time );
  return *this;
}

/*
 * The Appliance class is a simple scheduler for running State Machines
 *
 *********************************************************************************************
 *
 * Appliance::component( machine ) - Adds a State Machine object to the current appliance
 *
 */

Appliance& Appliance::component( Machine& machine ) {
  machine.inventory_next = inventory_root;
  inventory_root = &machine;
  return *this;
}

/*
 * Appliance::run( time ) - Runs the appliance
 *
 * If the 'time' argument is given, loops until that time has passed
 * otherwise executes only one cycle of each machine and exits
 *
 */

Appliance& Appliance::run( uint32_t time /* = 0 */ )  // Is it safe to allow recursion here???
{
  Machine* m;
  uint32_t cycle_start = millis();
  do {
    m = inventory_root;
    while ( m ) {
      if ( ( m->flags & ( ATM_SLEEP_FLAG | ATM_CYCLE_FLAG ) ) == 0 ) m->cycle();
      // Move to the next machine
      m = m->inventory_next;
      // if ( time > 0 && ( millis() - cycle_start ) < time ) break;
    }
  } while ( millis() - cycle_start < time );
  return *this;
}
