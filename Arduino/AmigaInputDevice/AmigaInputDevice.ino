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
//
//  This is distributed under the MIT license.
//  No warranty blah blah blah.


#define VERSTRING "004 2015-0704"
// version history
//
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
	9	R But	Button 2	D8

	Keyboard (TBD)

	NOTE: Also tie D6, D7, D8 to VCC via 10k Ohm resistor (tentative)

	NOTE: for Atari mouse, signals on D9 pins 1 and 4 functionality are swapped

*/
////////////////////////////////////////////////////////////////////////////////


// pin configurations
//	Mouse X and Y quadrature pins
#define kMouseXa (5)
#define kMouseXb (3)
#define kMouseYa (6)
#define kMouseYb (4)

// Atari mouse
#define kAtariMouseXa (5)
#define kAtariMouseXb (6)
#define kAtariMouseYa (3)
#define kAtariMouseYb (4)


//	Mouse buttons
#define kMouseB1 (7)
#define kMouseB2 (8)
#define kMouseB3 (2)

//	Joystick pins
#define kJoyUp    (6)
#define kJoyDown  (5)
#define kJoyLeft  (4)
#define kJoyRight (3)

// on-board LED pin
#define kLED (17)

// usage modes
#define kModeAmigaMouse (0) /* amiga mouse quad signals, to HID mouse */
#define kModeAtariMouse (1) /* atari mouse quad signals, to HID mouse */
#define kModeJoyMouse   (2) /* joystick sends LRUD, convert to mouse */
#define kModeJoyWASD    (3) /* joystick sends LRUD, convert to Arrow keys */
#define kModeJoyArrows  (4) /* joystick sends LRUD, convert to Arrow keys */

int mode = kModeAmigaMouse;

// ----------------------------------------

// setup - initialize the hardware
void setup() {
  pinMode( kLED, OUTPUT );
  
  // put your setup code here, to run once:
  Serial.begin( 9600 );
  
  initGrayMouse();   // use this for an Amiga Mouse
//  initJoystick();  // use this for a digital joystick (Atari Joystick)
  Mouse.begin();
  
  Serial.begin( 9600 );
  
  for( int i=0 ; i<5 ; i++ )
  {
    digitalWrite( kLED, HIGH ); delay( 50 );
    digitalWrite( kLED, LOW ); delay( 50 );
  }
}



// mouse movement history, used for acceleration
char history_x[128];
char history_y[128];
int historyPos=0; // current write position in the history

// initialize for gray code mouse 
void initGrayMouse( void )
{
  // set the mouse and button inputs
  pinMode( kMouseXa, INPUT );
  pinMode( kMouseXb, INPUT );
  pinMode( kMouseYa, INPUT );
  pinMode( kMouseXb, INPUT );
  pinMode( kMouseB1, INPUT_PULLUP );
  pinMode( kMouseB2, INPUT_PULLUP );
  pinMode( kMouseB3, INPUT_PULLUP );
  
  // set the mode
  mode = kModeAmigaMouse;
  
  // clear the history
  for( int h=0 ; h<128 ; h++ ) {
    history_x[h] = history_y[h] = 0;
  }
}

// initialize for gray code mouse 
void initJoystick( void )
{
  // set the mouse and button inputs
  pinMode( kJoyUp, INPUT_PULLUP );
  pinMode( kJoyDown, INPUT_PULLUP );
  pinMode( kJoyLeft, INPUT_PULLUP );
  pinMode( kJoyRight, INPUT_PULLUP );
  pinMode( kMouseB1, INPUT_PULLUP );
  pinMode( kMouseB2, INPUT_PULLUP );
  pinMode( kMouseB3, INPUT_PULLUP );
  
  // set the mode
  mode = kModeJoyMouse;
  
  // clear the history
  for( int h=0 ; h<128 ; h++ ) {
    history_x[h] = history_y[h] = 0;
  }
}

void switchMode( int mmmm )
{
  mode = mmmm;
  switch( mode ) {
    case( kModeAmigaMouse ):
    case( kModeAtariMouse ):
      initGrayMouse();
      break;

    case( kModeJoyMouse ):
    case( kModeJoyWASD ):
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
  switch( mode ) {
    case( kModeAmigaMouse ):
      Serial.println( "Amiga mouse as HID mouse" );
      break;
    case( kModeAtariMouse ):
      Serial.println( "Atari mouse as HID mouse" );
      break;

    case( kModeJoyMouse ):
      Serial.println( "Atari joystick as HID mouse" );
      break;
      
    case( kModeJoyWASD ):
      Serial.println( "Atari joystick as WASD keys" );
      break;
      
    case( kModeJoyArrows ):
      Serial.println( "Atari joystick as Arrow keys" );
      break;
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
      Serial.println( " 0  Amiga mouse as HID mouse" );
      Serial.println( " 1  Atari mouse as HID mouse" );
      Serial.println( " 2  Atari joystick as HID mouse" );
      Serial.println( " 3  Atari joystick as WASD keyboard" );
      Serial.println( " 4  Atari joystick as arrow keys" );
      Serial.println( "" );
      Serial.println( " h  Help info." );
      Serial.println( " v  Version info." );
      Serial.println( " m  Display use mode." );
    }
    if( ch == 'm' ) {
      dumpMode();
    }
    if( ch == 'v' ) {
      Serial.println( "Amiga Input Device tool" );
      Serial.print( " v" );
      Serial.print( VERSTRING );
      Serial.println( " (c) 2016 yorgle@gmail.com" );
      Serial.println( " created for the 2015/07 http://retrochallenge.org" );
    }
    if( ch == '0' ) {
      switchMode( kModeAmigaMouse );
      Serial.println( "Selected 0: Amiga mouse -> HID Mouse" );
    }
    if( ch == '1' ) {
      switchMode( kModeAtariMouse );
      Serial.println( "Selected 1: Atari mouse -> HID Mouse" );
    }
    if( ch == '2' ) {
      switchMode( kModeJoyMouse );
      Serial.println( "Selected 2: Atari Joystick -> HID Mouse" );
    }
    if( ch == '3' ) {
      switchMode( kModeJoyWASD );
      Serial.println( "Selected 3: Atari Joystick -> HID WASD Keyboard" );
    }
    if( ch == '4' ) {
      switchMode( kModeJoyArrows );
      Serial.println( "Selected 4: Atari Joystick -> HID Arrows Keyboard" );
    }
    
  }
}

 
// main loop
void loop() {
  if( Serial.available() ) {
    serialShell();
  }
  switch( mode ) {
    case( kModeAmigaMouse ):
    case( kModeAtariMouse ):
      loopGrayMouse();
      break;
    case( kModeJoyMouse ):
      loopJoyMouse();
      break;
    case( kModeJoyWASD ):
    default:
      break;
  }
}

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
  
  if( mode == kModeAmigaMouse ) {
    // amiga mouse
    hq = (digitalRead( kMouseXa ) << 1) | digitalRead( kMouseXb );
    vq = (digitalRead( kMouseYa ) << 1) | digitalRead( kMouseYb );
    
  } else if( mode == kModeAtariMouse ) {
    // atari mouse ( Xb, Ya swapped wrt Amiga mouse )
    hq = (digitalRead( kAtariMouseXa ) << 1) | digitalRead( kAtariMouseXb );
    vq = (digitalRead( kAtariMouseYa ) << 1) | digitalRead( kAtariMouseYb );
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

void loopJoyMouse()
{
  int u = digitalRead( kJoyUp )?0:-1;
  int d = digitalRead( kJoyDown )?0:1;
  int l = digitalRead( kJoyLeft )?0:-1;
  int r = digitalRead( kJoyRight )?0:1;
  
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
