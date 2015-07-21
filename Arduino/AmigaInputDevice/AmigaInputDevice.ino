// AmigaInputDevice
//
//  yorgle@gmail.com
//  http://geodesicsphere.blogspot.com
//  http://yorgle.org
//
//  A program for the Arduino Leonardo and other ATmega32u4 based 
//  devices to read in an Amiga Mouse and Joystick and present them
//  as HID USB mouse and keyboard.
//
//  This file should be available through the github repository at
//	https://github.com/BleuLlama/AmigaInputToUSB
//
//  Wiring diagram and such will be available there in image form
//e
//  This is distributed under the MIT license.
//  No warranty blah blah blah.


#define VERSTRING "007 2015-0720"
// version history
//
// v 007  2015-07-20  Atari ST Mouse support tested.  Pin define cleanups
// v 006  2015-07-13  Joystick-Keypress support complete with various preset configurations
// v 005  2015-07-12  EEprom saving of settings
// v 004  2015-07-04  Serial Control Shell
// v 003  2015-07-03  Pinout updated to mtch v1 hardware
// v 002  2015-07-02  L/R/M mouse buttons, modes, Mouse button fix, simple acceleration
// v 001  2015-07-02  initial version
//

////////////////////////////////////////////////////////////////////////////////
/*
	hookup info (tentative)

		Amiga	Atari
	D9 pin	Mouse	Joystick	Arduino Pin

	1	V	Forward		D6
	2	H	Backward	D5
	3	VQ	Left		D4
	4	HQ	Right		D3
	5	M But	n/c		D2

	6	L But	Button 1	D7
	7	+5V			(VCC)
	8	GND			(GND)
	9	R But	Button 2	D14

	Keyboard (TBD)

	NOTE: Also tie D6, D7, D8 to VCC via 10k Ohm resistor (tentative)

	NOTE: for Atari mouse, signals on D9 pins 1 and 4 functionality are swapped (in software)
*/
////////////////////////////////////////////////////////////////////////////////

#include <EEPROM.h>

// how the D9 is connected to data pins...
#define kD9_1  (6)
#define kD9_2  (5)
#define kD9_3  (4)
#define kD9_4  (3)
#define kD9_5  (2)
#define kD9_6  (7)
/* pins 7,8 are connected to power, ground */
#define kD9_9  (14) 

// pin configurations (how it's wired to logical signal lines...)
// Amiga Mouse
#define kMouseAmigaV  (kD9_1)
#define kMouseAmigaH  (kD9_2)
#define kMouseAmigaVQ (kD9_3)
#define kMouseAmigaHQ (kD9_4)

// Atari mouse
#define kAtariMouseXb (kD9_1)
#define kAtariMouseXa (kD9_2)
#define kAtariMouseYa (kD9_3)
#define kAtariMouseYb (kD9_4)

// Mouse buttons (common for both)
#define kMouseB1 (kD9_6)  /* left button */
#define kMouseB2 (kD9_9)  /* right button */
#define kMouseB3 (kD9_5)  /* middle button */

// Joystick pins
#define kJoyUp    (kD9_1)
#define kJoyDown  (kD9_2)
#define kJoyLeft  (kD9_3)
#define kJoyRight (kD9_4)

// on-board LED pin
#define kLED (17)

/////////////////////////////
#define kNSettings (8)
char settings[kNSettings];

// 0..2 are 'S' 'D' 'L' (Sentinel)
#define kSettingVers (3) // settings version
#define kSettingMode (4) // D9 Port usage mode

/////////////////////////////
// usage modes
#define kModeAmigaMouse (0) /* amiga mouse quad signals, to HID mouse */
#define kModeAtariMouse (1) /* atari mouse quad signals, to HID mouse */
#define kModeJoyMouse   (2) /* joystick sends LRUD, convert to mouse */

                            /* joystick sends LRUD... */
#define kModeJoyFSUAE   (3) /* convert to arrow keys, right ctrl/alt */
#define kModeJoyMame    (4) /* convert to arrow keys, left ctrl/alt */
#define kModeJoyStella  (5) /* convert to arrow keys, space, 4, 5 */
#define kModeJoyWASD    (6) /* convert to WASD keys */
#define kModeJoyHJKL    (7) /* convert to vi hjkl keys */

#define kModeExplore    (8) /* Exploration of ideas... */
#define kModeMax (kModeExplore)


// ----------------------------------------

// setup - initialize the hardware
void setup() {
  pinMode( kLED, OUTPUT );
  
    // set the mouse and button inputs
  pinMode( kD9_1, INPUT_PULLUP );
  pinMode( kD9_2, INPUT_PULLUP );
  pinMode( kD9_3, INPUT_PULLUP );
  pinMode( kD9_4, INPUT_PULLUP );
  pinMode( kD9_5, INPUT_PULLUP );
  pinMode( kD9_6, INPUT_PULLUP );
  pinMode( kD9_9, INPUT_PULLUP );
  
  // put your setup code here, to run once:
  Serial.begin( 9600 );
  Mouse.begin(); // for USB HID Mouse support
  Keyboard.begin(); // for USB HID Keyboard support
  
  loadSettings();
}

void loadSettings( void )
{
  initGrayMouse();   // use this for an Amiga Mouse
//  initJoystick();  // use this for a digital joystick (Atari Joystick)

  for( int i=0 ; i<kNSettings ; i++ ) {
    settings[i] = EEPROM.read( i );
  }
  
  // autoinit
  if( settings[0] != 'S' || settings[1] != 'D' || settings[2] != 'L' ) {
    defaultSettings();
  }
  
  // configure mode
  switchMode( settings[kSettingMode] );
}

void defaultSettings( void )
{
  settings[0] = 'S'; // sentinel
  settings[1] = 'D';
  settings[2] = 'L';
  settings[3] = 1;   // 
  for( int s = 4 ; s < kNSettings ; s ++ ) {
    settings[s] = 0;
  }
  saveSettings();
}

void saveSettings( void )
{
  for( int i=0 ; i<kNSettings ; i++ ) {
    EEPROM.write( i, settings[i] );
  }
}

void dumpSetting( int i, char * info )
{
  Serial.print( "S." );
  Serial.print( i, DEC );
  Serial.print( ": " );
  Serial.print( settings[i], HEX );
  Serial.print( "  EE:" );
  Serial.print( EEPROM.read( i ), HEX );
  Serial.println( info );
}

void dumpSettings( void )
{
  dumpSetting( 0, "  'S' 0x53 (sentinel)" );
  dumpSetting( 1, "  'D' 0x44 (sentinel)" );
  dumpSetting( 2, "  'L' 0x4c (sentinel)" );
  dumpSetting( 3, "  version" );
  dumpSetting( 4, "  input mode:" );
  dumpMode();
}

// mouse movement history, used for acceleration
char history_x[128];
char history_y[128];
int historyPos=0; // current write position in the history

// initialize for gray code mouse 
void initGrayMouse( void )
{
  // clear the history
  for( int h=0 ; h<128 ; h++ ) {
    history_x[h] = history_y[h] = 0;
  }
}

// initialize for gray code mouse 
void initJoystick( void )
{
  // clear the history
  for( int h=0 ; h<128 ; h++ ) {
    history_x[h] = history_y[h] = 0;
  }
}

void switchMode( int mmmm )
{
  settings[kSettingMode] = mmmm;
  saveSettings();
  Keyboard.releaseAll();
  
  switch( settings[kSettingMode] ) {
    case( kModeAmigaMouse ):
    case( kModeAtariMouse ):
      initGrayMouse();
      break;

    case( kModeJoyFSUAE ):
    case( kModeJoyMame ):
    case( kModeJoyStella ):
    case( kModeJoyWASD ):
    case( kModeJoyHJKL ):
    case( kModeExplore ):
      initJoystick();
      break;
  }
}


// provide a total of all X history values (for acceleration)
int total_x( void )
{
  int ret = 0;
  for( int i=0 ; i < 128 ; i++ )
  {
    ret += history_x[i];
  }
  return ret;
}

// provide a total of all Y history values (for acceleration)
int total_y( void )
{
  int ret = 0;
  for( int i=0 ; i < 128 ; i++ )
  {
    ret += history_y[i];
  }
  return ret;
}


// compare A and B to determine the delta
  // +  00 -> 01 -> 11 -> 10 -> 00 
  // -  00 -> 10 -> 11 -> 01 -> 00
int grayCompare( int a, int b )
{
  a = a & 0x03;
  b = b & 0x03;
  if( a == b ) return 0;
  switch( a ) {
    case( 0x00 ):
      if( b == 0x01) return +1;
      if( b == 0x10) return -1;
      break;
    case( 0x01 ):
      if( b == 0x11) return +1;
      if( b == 0x00) return -1;
      break;
    case( 0x11 ):
      if( b == 0x10) return +1;
      if( b == 0x01) return -1;
      break;
    case( 0x10 ):
      if( b == 0x00) return +1;
      if( b == 0x11) return -1;
      break;
  }
  return 0;
}

void dumpMode()
{
  switch( settings[kSettingMode] ) {
    // -> Mouse
    case( kModeAmigaMouse ): Serial.println( "Amiga mouse as HID mouse" ); break;
    case( kModeAtariMouse ): Serial.println( "Atari mouse as HID mouse" ); break;
    case( kModeJoyMouse ):   Serial.println( "Atari joystick as HID mouse" ); break;
    
    // -> Keyboard
    case( kModeJoyFSUAE ):   Serial.println( "Atari Joystick as Arrow/r-ctrl/r-alt (FS-UAE, Vice)" ); break;
    case( kModeJoyMame ):    Serial.println( "Atari Joystick as Arrow/l-ctrl/l-alt (Mame)" ); break;
    case( kModeJoyStella ):  Serial.println( "Atari Joystick as Arrow/ /4/5 (Stella)" ); break;
    case( kModeJoyWASD ):    Serial.println( "Atari joystick as WASD keys" ); break;
    case( kModeJoyHJKL ):    Serial.println( "Atari joystick as vi (hjkl) keys" ); break;
    
    case( kModeExplore ):    Serial.println( "Controller explorer" ); break;
    default: break;
  }
}

// stupid serial shell to change usage mode
void serialShell()
{
  while( Serial.available() ) {
    int ch = Serial.read();
    if( ch == '\n' ) {
      Serial.println( "\n> " );
      Serial.flush();
    }
    if( ch == '?' || ch == 'h' ) {
      Serial.println( "Select mode (type the digit.)" );
      Serial.println( " 0  USB Mouse output from Amiga mouse" );
      Serial.println( " 1  USB Mouse output from Atari mouse" );
      Serial.println( " 2  USB Mouse output from Atari joystick" );
      
      Serial.println( " 3  Joystick as FS-UAE/Vice Keyboard (arrows/r-ctrl/r-alt)" );
      Serial.println( " 4  Joystick as MAME Keyboard (arrows/l-ctrl/l-alt)" );
      Serial.println( " 5  Joystick as Stella Keyboard (arrows/ /4/5)" );
      Serial.println( " 6  Joystick as WASD Keyboard (w/s/a/d)" );
      Serial.println( " 7  Joystick as vi Keyboard (h/j/k/l)" );
      
      Serial.println( " 8  Controller Explorer" );

      Serial.println( "" );
      Serial.println( "Other options:" );
      Serial.println( " h  Help info." );
      Serial.println( " v  Version info." );
      Serial.println( " m  Display use mode." );
      Serial.println( " d  Dump settings from EEPROM." );
      Serial.println( " i  initialize settings in EEPROM." );
    }
    if( ch == 'i' ) {
      defaultSettings();
    }
    if( ch == 'd' ) {
      dumpSettings();
    }
    if( ch == 'm' ) {
      dumpMode();
    }
    if( ch == 'v' ) {
      Serial.println( "Amiga Input Device tool" );
      Serial.print( " v" );
      Serial.print( VERSTRING );
      Serial.println( " (c) 2015 yorgle@gmail.com" );
      Serial.println( " created for the 2015/07 http://retrochallenge.org" );
    }
    
    if( ch >= ('0' + kModeAmigaMouse)  && ch <= ('0' + kModeMax ) ) {
      switchMode( ch - '0' );
      dumpMode();
    }    
  }
}

 
//////////////////////////////////////////////////////
// main loop
void loop() {
  if( Serial.available() ) {
    serialShell();
  }
  switch( settings[kSettingMode] ) {
    case( kModeAmigaMouse ):
    case( kModeAtariMouse ):
      loopGrayMouse();
      break;
    case( kModeJoyMouse ):
      loopJoyMouse();
      break;
    case( kModeJoyFSUAE ):
    case( kModeJoyMame ):
    case( kModeJoyStella ):
    case( kModeJoyWASD ):
    case( kModeJoyHJKL ):
      loopJoyKeys();
      break;
    
    case( kModeExplore ):
      loopExplore();
      break;
      
    default:
      break;
  }
}

//////////////////////////////////////////////////////
// handler to read the mouse buttons.
// broke this out since it's the same for mouse vs joystick
void handleButtonPresses()
{
  // b1 = left
  // b2 = right
  // b3 = middle
  
  // left
  static int lb1 = LOW;
  int b1 = digitalRead( kMouseB1 );
  
  if( b1 != lb1 ) {
    if( b1 == LOW ) {
      Mouse.press( MOUSE_LEFT );
      delay( 50 ); // fakeo debounce
    }
    if( b1 == HIGH ) Mouse.release( MOUSE_LEFT );
  }
  
  // necessary to prevent mouse fighting
  //if( b1 == HIGH && Mouse.isPressed( MOUSE_LEFT )) {
  //  Mouse.release( MOUSE_LEFT );
  //}
  
  lb1 = b1;

  
  // right
  static int lb2 = LOW;
  int b2 = digitalRead( kMouseB2 );
  
  if( b2 != lb2 ) {
    if( b2 == LOW ) {
      Mouse.press( MOUSE_RIGHT );
      delay( 50 ); // fakeo debounce
    }
    if( b2 == HIGH ) Mouse.release( MOUSE_RIGHT );
  }
  // necessary to prevent mouse fighting
  //if( b2 == HIGH && Mouse.isPressed( MOUSE_RIGHT )) {
  //  Mouse.release( MOUSE_RIGHT );
  //}
  lb2 = b2;
  
  
  // middle
  static int lb3 = LOW;
  int b3 = digitalRead( kMouseB3 );
  
  if( b3 != lb3 ) {
    if( b3 == LOW ) {
      Mouse.press( MOUSE_MIDDLE );
      delay( 50 ); // fakeo debounce
    }
    if( b3 == HIGH ) Mouse.release( MOUSE_MIDDLE );
  }
  // necessary to prevent mouse fighting
  //if( b3 == HIGH && Mouse.isPressed( MOUSE_MIDDLE )) {
  //  Mouse.release( MOUSE_MIDDLE );
  //}
  lb3 = b3;
}


//////////////////////////////////////////////////////
// quadrature gray as HID mouse

// the main gray mouse loop
void loopGrayMouse()
{
  bool changed = false; // did something change?
  
  // these are static so they persist between calls
  static int x_accum = 0; // accumulated X
  static int y_accum = 0; // accumulated Y

  static int last_hq = 0; // last h quadrature
  static int last_vq = 0; // last v quadrature
  
  // read in the quad/gray code
  int hq = 0;
  int vq = 0;
  
  if( settings[kSettingMode] == kModeAmigaMouse ) {
    // amiga mouse
    hq = (digitalRead( kMouseAmigaH ) << 1) | digitalRead( kMouseAmigaHQ );
    vq = (digitalRead( kMouseAmigaV ) << 1) | digitalRead( kMouseAmigaVQ );
    
  } else if( settings[kSettingMode] == kModeAtariMouse ) {
    // atari mouse ( Xb, Ya swapped wrt Amiga mouse )
    hq = (digitalRead( kAtariMouseXa ) << 1) | digitalRead( kAtariMouseXb );
    vq = (digitalRead( kAtariMouseYb ) << 1) | digitalRead( kAtariMouseYa );  /* vertical swapped! */
  }
  
  // check horizontal delta
  if( hq != last_hq ) {
    x_accum += grayCompare( hq, last_hq );
    changed = true;
  }
  last_hq = hq;
  
  // and vertical delta
  if( vq != last_vq ) {
    y_accum += grayCompare( vq, last_vq );
    changed = true;
  }
  last_vq = vq;
  
  
  // generate the acceleration info from gray tick
  historyPos++;
  history_x[ (historyPos & 0x7f) ] = x_accum;
  history_y[ (historyPos & 0x7f) ] = y_accum;
  int tx = total_x() * 2;
  int ty = total_y() * 2;
  
  // if something changed, move the mouse
  if( x_accum || y_accum ) {
    Mouse.move( tx, ty, 0 );
    x_accum = y_accum = 0;
  }
  
  // and check the mouse buttons too
  handleButtonPresses();
}


//////////////////////////////////////////////////////
// Joystick as mouse
void loopJoyMouse()
{
  char u = digitalRead( kJoyUp )?0:-1;
  char d = digitalRead( kJoyDown )?0:1;
  char l = digitalRead( kJoyLeft )?0:-1;
  char r = digitalRead( kJoyRight )?0:1;
  
  historyPos++;
  history_x[ (historyPos & 0x7f) ] = l+r;
  history_y[ (historyPos & 0x7f) ] = u+d;
  int tx = total_x() / 24;
  int ty = total_y() / 24;

  if( u | d | r | l ) { 
    Mouse.move( tx, ty, 0 );
  }
  
  // and check the mouse buttons too
  handleButtonPresses();
}

// compare now with previous.
// send the key press, release, and return the current (to be assigned to previous)
char keyHelper( char now, char previous, char sendKey )
{
  if( !now && previous ) {
    // press event
    Keyboard.press( sendKey );
  }
  if( now && !previous ) {
    // release event
    Keyboard.release( sendKey );
  }
  
  return now;
}

void loopJoyKeys()
{
  static char lastU = 0;
  static char lastD = 0;
  static char lastL = 0;
  static char lastR = 0;
  static char lastB1 = 0;
  static char lastB2 = 0;
  static char lastB3 = 0;
  char u = digitalRead( kJoyUp );
  char d = digitalRead( kJoyDown );
  char l = digitalRead( kJoyLeft );
  char r = digitalRead( kJoyRight );
  char b1 = digitalRead( kMouseB1 );
  char b2 = digitalRead( kMouseB2 );
  char b3 = digitalRead( kMouseB3 );

  // ref: https://www.arduino.cc/en/Reference/KeyboardModifiers
  char* moves = "UDLR123"; // filler
  char movesWASD[]   = { 'w', 's', 'a', 'd', ' ', ' ', ' ' };  
  char movesHJKL[]   = { 'k', 'j', 'h', 'l', KEY_ESC, KEY_ESC, KEY_ESC };
  char movesStella[] = { KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_LEFT_ARROW, KEY_RIGHT_ARROW, ' ', '4', '5' };
  char movesMame[]   = { KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_LEFT_ARROW, KEY_RIGHT_ARROW, KEY_LEFT_CTRL, KEY_LEFT_ALT, ' ' };
  char movesFSUAE[]  = { KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_LEFT_ARROW, KEY_RIGHT_ARROW, KEY_RIGHT_CTRL, KEY_RIGHT_ALT, ' ' };

  if( settings[kSettingMode] == kModeJoyWASD ) moves = movesWASD;
  if( settings[kSettingMode] == kModeJoyHJKL ) moves = movesHJKL;
  if( settings[kSettingMode] == kModeJoyStella ) moves = movesStella;
  if( settings[kSettingMode] == kModeJoyMame ) moves = movesMame;
  if( settings[kSettingMode] == kModeJoyFSUAE ) moves = movesFSUAE;

  lastU = keyHelper( u, lastU, moves[0] );
  lastD = keyHelper( d, lastD, moves[1] );
  lastL = keyHelper( l, lastL, moves[2] );
  lastR = keyHelper( r, lastR, moves[3] );
  lastB1 = keyHelper( b1, lastB1, moves[4] );
  lastB2 = keyHelper( b2, lastB2, moves[5] );
  lastB3 = keyHelper( b3, lastB3, moves[6] );
}

//////////////////////////////////////////////////////

bool isJoystick = true;
bool maybeAtari = false;
bool maybeAmiga = false;

void loopExplore()
{
  static int nTransitions = 0;
  static char lastData = 0xff;
  char data = 0x00;

  // read all 4 bits
  char u = digitalRead( kJoyUp )   ? 0 : 1;
  char d = digitalRead( kJoyDown ) ? 0 : 1;
  char l = digitalRead( kJoyLeft ) ? 0 : 1; 
  char r = digitalRead( kJoyRight )? 0 : 1;
  
  // combine them so we can easily detect change
  data = u | (d<<1) | (l<<2) | (r<<3);
  
  if( data != lastData ) {
    Serial.print( nTransitions++ );
    Serial.print( ": " );

    // atari pairings
    Serial.print( u, DEC );
    Serial.print( d, DEC );
    Serial.print( " " );
    Serial.print( l, DEC );
    Serial.print( r, DEC );

    // amiga pairings
    Serial.print( "  " );
    Serial.print( r, DEC );
    Serial.print( d, DEC );
    Serial.print( " " );
    Serial.print( l, DEC );
    Serial.print( u, DEC );

    
    // up+down or left+right are not possible on a joystick
    if( u && d ) isJoystick = false;
    if( l && r ) isJoystick = false;
    
    Serial.print( "  " );
    if( isJoystick ) Serial.print( " Joystick  " );
    if( maybeAtari ) Serial.print( " Atari " );
    if( maybeAmiga ) Serial.print( " Amiga " );
    if( !isJoystick ) Serial.print( " Mouse" );
    Serial.println();
    lastData = data;
  }
}
