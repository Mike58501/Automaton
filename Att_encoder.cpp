#include "Att_encoder.hpp"

const char Att_encoder::_enc_states[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

Att_encoder& Att_encoder::begin( int pin1, int pin2, int divider /* = 4 */ ) {
  // clang_format off
  const static STATE_TYPE state_table[] PROGMEM = {/*          ON_ENTER     ON_LOOP  ON_EXIT  EVT_UP  EVT_DOWN  ELSE */
                                                   /* IDLE  */ -1,       ACT_SAMPLE, -1, UP, DOWN, -1,
                                                   /* UP    */ ACT_UP,   -1,         -1, -1, -1,   IDLE,
                                                   /* DOWN  */ ACT_DOWN, -1,         -1, -1, -1,   IDLE,
  };
  // clang_format on
  MACHINE::begin( state_table, ELSE );
  _pin1 = pin1;
  _pin2 = pin2;
  _divider = divider;
  pinMode( _pin1, INPUT );
  pinMode( _pin2, INPUT );
  digitalWrite( _pin1, HIGH );  // Is this needed?
  digitalWrite( _pin2, HIGH );
  return *this;
}

Att_encoder& Att_encoder::onUp( Machine& machine, int event /* = 0 */ ) {
  _onup.set( &machine, event );
  return *this;
}

Att_encoder& Att_encoder::onUp( TinyMachine& machine, int event /* = 0 */ ) {
  _onup.set( &machine, event );
  return *this;
}

Att_encoder& Att_encoder::onUp( atm_cb_t callback, int idx /* = 0 */ ) {
  _onup.set( callback, idx );
  return *this;
}

Att_encoder& Att_encoder::onUp( const char* label, int event /* = 0 */ ) {
  _onup.set( label, event );
  return *this;
}

Att_encoder& Att_encoder::onDown( Machine& machine, int event /* = 0 */ ) {
  _ondown.set( &machine, event );
  return *this;
}

Att_encoder& Att_encoder::onDown( TinyMachine& machine, int event /* = 0 */ ) {
  _ondown.set( &machine, event );
  return *this;
}

Att_encoder& Att_encoder::onDown( atm_cb_t callback, int idx /* = 0 */ ) {
  _ondown.set( callback, idx );
  return *this;
}

Att_encoder& Att_encoder::onDown( const char* label, int event /* = 0 */ ) {
  _ondown.set( label, event );
  return *this;
}

int Att_encoder::event( int id ) {
  switch ( id ) {
    case EVT_UP:
      return _enc_direction == 1 && ( _enc_counter % _divider == 0 );
    case EVT_DOWN:
      return _enc_direction == -1 && ( _enc_counter % _divider == 0 );
  }
  return 0;
}

void Att_encoder::action( int id ) {
  switch ( id ) {
    case ACT_SAMPLE:
      _enc_bits = ( ( _enc_bits << 2 ) | ( digitalRead( _pin1 ) << 1 ) | ( digitalRead( _pin2 ) ) ) & 0x0f;
      if ( ( _enc_direction = _enc_states[_enc_bits] ) != 0 ) {
        _enc_counter++;
      }
      return;
    case ACT_UP:
      _onup.push( FACTORY );
      return;
    case ACT_DOWN:
      _ondown.push( FACTORY );
      return;
  }
}

Att_encoder& Att_encoder::trace( Stream& stream ) {
  Machine::setTrace( &stream, atm_serial_debug::trace, "EVT_UP\0EVT_DOWN\0ELSE\0IDLE\0UP\0DOWN" );
  return *this;
}