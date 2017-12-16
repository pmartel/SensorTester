/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

  The circuit:
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

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
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

#define RECEIVER_PIN 6 // Signal Pin of IR receiver to Arduino Digital Pin 6

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
    case 1:
      lcd.clear();
      break;
    case 2:
      nyi( String(mode) );
      break;
    case 3:
      nyi( String(mode) );
      break;
    default:
      mode = 0;
      break;
    }
  }

  switch ( mode ) { //process current mode
  case 1:
    doRange(); 
    break;
  case 2:
    break;
  case 3:
    break;
  default:
    break;
  }

  if (irrecv.decode(&results)) { // have we received an IR signal?
    translateIR(); 
    irrecv.resume(); // receive the next value
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
    mode = ret = 0;   
    break;
  case 0xFF30CF: 
    Serial.println("1"); 
    mode = ret = 1;   
    break;
  case 0xFF18E7: 
    Serial.println("2"); 
    mode = ret = 2;   
    break;
  case 0xFF7A85: 
    Serial.println("3"); 
    mode = ret = 3;   
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
    9;   
    break;
  case 0xFFFFFFFF: 
    Serial.println(" REPEAT"); // button held down
    break;  
  default: 
    Serial.println(" other button   ");

  }// End Case

  delay(500); // Do not get immediate repeat
  return ret;
} //END translateIR

