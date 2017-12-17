/*
 Sensor tester.
 Uses the Elegoo IR transmitter/receiver to pass commands to the tester
 * IR receiver to digital pin 6 
 Uses the 16x2 LCD display to display results.

  The LDC circuit:
 * (Put these in order
 * LCD VSS pin to ground
 * LCD VCC pin to 5V (should be VDD - POM)
 * 10K pot:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 * LCD RS pin to digital pin 7
 * LCD R/W pin to ground
 * LCD E(nable) pin to digital pin 8
 * LCD D4 pin to digital pin 9
 * LCD D5 pin to digital pin 10
 * LCD D6 pin to digital pin 11
 * LCD D7 pin to digital pin 12
 * LCD A pin through 220 Ohm resistor to 5V (added -POM)
 * LCD K pin to ground (added -POM)

  The range finder is an SR-04 type
 * trigger pin to digital pin 2
 * echo pin to digital pin 3

 * Encoder / chronograph
 * Chan A to digital pin 0
 * Chan B to digital pin 1
 */

/* TO-DO: 
 *  add code to format a string into a character array and to display
 *  the array on the LCD using the UP and DOWN arrows to scroll through the
 *  array.
 */ 
 
 // include the library code:
#include "IRremote.h"
#include <LiquidCrystal.h>
#include "SR04.h" // range finder
#include "Streaming.h" // C++ style output

// initialize the LCD library with the numbers of the interface pins
//               RS, E,D4, D5, D6, D7 
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// set up the range finder
#define TRIG_PIN 2
#define ECHO_PIN 3
SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);

// set up the IR receiver
#include "IRremote.h"

// Signal Pin of IR receiver to Arduino Digital Pin 6
#define RECEIVER_PIN 6 

// pins for encoder or ballixtic chronograph
// digital pins 0 and 1 interfere with the serial I/O, use the "analog" pins
#define CHAN_A_PIN  A0
#define CHAN_B_PIN  A1

/***********************************************************/
/* Think of having am array of characters, for now 8x16 and 
 *  having the LCD being a window into this array.  Keys
 *  can move the window up and down, and potentially left and right
 */

class Display {
  public:
    Display( int rows,int columns ){
      _rows = rows;
      _columns = columns;
      _display = malloc(rows *columns); 
    }
    Display(void){
      Display(8,16);
    }
    ~Display(){
      free(_display);
    }
    Clear(){
      int i;
      for (i=0; i < _rows * _columns; i++){
        _display[i] = " ";
      }
      Home();
    }
    Home(){
      _r = 0;
      _c = 0;
    }
    //load the string s starting at position (r,c)
    // special characters handled as noted
    // for now, wrap characters around
    Load( char* s, int row, int col ) {
      char c;
      
      _r = row;
      _c = col;
      // place character
      for ( ; *s != '\0'; s++ ) {
        c = *s;
        switch ( c ){
        case '\n':
          break;
        case '\r':
          break;
        case '\b':
          break;
        case '\t':
          break;
        default:
          break;
        }
        // update cursor
      }
      
    }
    Load( char* s ){
      Load( s, 0, 0);
    }
    
  private:
    // size
    int _rows;
    int _columns;
    // cursor position
    int _r;
    int _c;
    char* _display;
};

//Display display();

/*-----( Declare objects )-----*/
IRrecv irrecv(RECEIVER_PIN);     // create instance of 'IRrecv'
decode_results results;      // create instance of 'decode_results'

int mode = 0, oldMode = -1;

 void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  irrecv.enableIRIn(); // Start the receiver
  Serial.begin(115200);

}

void loop() {
  // see if mode has changed and display info
  
  if ( oldMode != mode ) { // changed mode
    switch (oldMode) { // close old mode if needed
    default:
      break;
    }
    oldMode = mode;
    // set up new mode
    switch (mode) {
    case 0: // display menu
      displayMenu(0);
      break;
    case 1:  // rangefinder
      lcd.clear();
      break;
    case 2: // quadrature encoder
      pinMode( CHAN_A_PIN, INPUT_PULLUP );
      pinMode( CHAN_B_PIN, INPUT_PULLUP );
      lcd.clear();
      break;
    case 3: //ballistic chronograph
      pinMode( CHAN_A_PIN, INPUT);
      pinMode( CHAN_B_PIN, INPUT);
      lcd.clear();
      break;
    default:
      mode = 0;
      break;
    }
  }

  switch ( mode ) { //process current mode
  case 1:  // rangefinder
    doRange(); 
    break;
  case 2: // quadrature encoder
    doEncoder();
    break;
  case 3: //ballistic chronograph
    doBallistic();
  default:
    break;
  }

  if (irrecv.decode(&results)) { // have we received an IR signal?
    int irData = translateIR(); 
    irrecv.resume(); // receive the next value
    // handle input
    if ( (irData >= 0) && ( irData <= 3 )) { //update mode
      mode = irData; 
    }
    else {
      
    }
  }  

} // END loop()

// auxilliary functions
void displayMenu( int first ) {
  lcd.clear();
  //      1234567890123456"
  lcd << "0:menu 1:range";
  lcd.setCursor(0,1);
  lcd << "2:quad 3:speed";
}

// note: this does not return until a shot passes
// to exit, select a different mode and then fire a shot (wave an arm, etc)
void doBallistic( void ){
  // wait for channel A to drop, time until channel B drops
  long tA, tB;
  float v;
  String vS;

#ifdef DEBUG
  // testing with a short to ground requires pullups
  // either in the Arduino or external
  pinMode( CHAN_A_PIN, INPUT_PULLUP );
  pinMode( CHAN_B_PIN, INPUT_PULLUP );
#endif 
 
  while( digitalRead( CHAN_A_PIN ) == HIGH ){};
  tA = micros();
  while( digitalRead( CHAN_B_PIN ) == HIGH ){};
  tB = micros();
  lcd.clear();
  v = 1.e6/(tB-tA);
  vS = String( v, 2);
  //      0123456789012345
  lcd << "Speed        FPS";
  lcd.setCursor( 12 - vS.length(), 0 );
  lcd << vS;
}

#define CHAR_L '-'
 /* vertical bar */
 #define CHAR_H '\x7C'
void doEncoder( void ){
  int chA, chB;

  chA = digitalRead( CHAN_A_PIN );
  chB = digitalRead( CHAN_B_PIN );
  lcd.home();
  lcd << "A: " << (chA ? CHAR_H : CHAR_L);
  lcd.setCursor(0,1);
  lcd << "B: " << (chB ? CHAR_H : CHAR_L);
}

void doRange( void ) {
  String s = String(sr04.Distance());
  
  lcd.home();
  //          0123456789012
  lcd.print( "Range      cm" );
  lcd.setCursor(10 - s.length(),0);
  lcd.print(s);
  delay(500);  
}

void nyi( String s ) {
  lcd.clear();
  lcd << "mode " << s << " not yet implemented";
}

// This codebased on the Elegoo listings.  The case values are magic numbers
// that correspond to the IR transmitter codes
// the return value is 0-9 for the number keys and 10-20 for the others
// 10 is power, 11 is vol+ and so on until 20 is st/rep

int translateIR(void) { // takes action based on IR code received
  int ret = -1;
    
  switch(results.value) {
  case 0xFFA25D: 
    Serial.println("POWER"); 
    ret = 10;
    break;
  case 0xFF629D: 
    Serial.println("VOL+"); 
    ret = 11;
    break;
  case 0xFFE21D: 
    Serial.println("FUNC/STOP"); 
    ret = 12;
    break;
  case 0xFF22DD: 
    ret = 13;
    Serial.println("FAST BACK");    
    break;
  case 0xFF02FD: 
    Serial.println("PAUSE");    
    ret = 14;
    break;
  case 0xFFC23D: 
    Serial.println("FAST FORWARD");   
    ret = 15;
    break;
  case 0xFFE01F: 
    Serial.println("DOWN");    
    ret = 16;
    break;
  case 0xFFA857: 
    Serial.println("VOL-");    
    ret = 17;
    break;
  case 0xFF906F: 
    Serial.println("UP");    
    ret = 18;
    break;
  case 0xFF9867: 
    Serial.println("EQ");    
    ret = 19;
    break;
  case 0xFFB04F: 
    Serial.println("ST/REPT");    
    ret = 20;
    break;
  case 0xFF6897: 
    Serial.println("0"); 
    ret = 0;   
    break;
  case 0xFF30CF: 
    Serial.println("1"); 
    ret = 1;   
    break;
  case 0xFF18E7: 
    Serial.println("2"); 
    ret = 2;   
    break;
  case 0xFF7A85: 
    Serial.println("3"); 
    ret = 3;   
    break;
  case 0xFF10EF: 
    Serial.println("4");
    ret = 4;    
    break;
  case 0xFF38C7: 
    Serial.println("5");
    ret = 5;    
    break;
  case 0xFF5AA5: 
    Serial.println("6");
    ret = 6;    
    break;
  case 0xFF42BD: 
    Serial.println("7");
    ret = 7;
    break;
  case 0xFF4AB5: 
    Serial.println("8");
    ret = 8;    
    break;
  case 0xFF52AD: 
    Serial.println("9");
    ret = 9;   
    break;
  case 0xFFFFFFFF: 
    Serial.println(" REPEAT"); // button held down
    ret = 21;
    break;  
  default: 
    Serial.println(" other button   ");

  }// End Case

  delay(500); // Do not get immediate repeat
  return ret;
} //END translateIR


