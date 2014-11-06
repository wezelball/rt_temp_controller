/* Dave Cohen
 * An improved radio telescope temperature controller
 * with a cool KS0108 LCD display and an Arduino Mega
 * 2560 controller
 * 11/5/2014
 *
 *
 * 0,0 is the upper left corner
 * 128, 64 is lower right corner
 */
 
#include <glcd.h>
#include "fonts/Arial14.h"         // proportional font
#include "fonts/SystemFont5x7.h"   // system font
#include "bitmaps/ArduinoIcon.h"   // bitmap 


/* constants (thou art not evil) */
const float pi = 3.1416;  // good ole pi
const int SCREEN_WIDTH = 128;    // KS0108 LCD max screen width x
const int SCREEN_HEIGHT = 64;    // KS0108 LCD max screen height y

/* global variables (thou art evil) */
unsigned int x = 0;
//float y = 0.0;
float y[360];  // this is the y array

void setup()  {
  GLCD.Init(NON_INVERTED);     // initialise the library, non inverted writes pixels onto a clear screen
  GLCD.ClearScreen();
  GLCD.DrawBitmap(ArduinoIcon, 32,0, BLACK); //draw the bitmap at the given x,y position
  GLCD.ClearScreen();
  Serial.begin(9600);  // open the serial port for debugging
}

/* Converts actual X value to screen coordinate
 * for plotting. We'll start simple and plot
 * a sine function from y - 1 to -1 and 
 * x = 0 to 360
 */
int xToScreen(float x) {
  return (int(x * (SCREEN_WIDTH/360.0)));
}

int yToScreen(float y) {
  return(int(SCREEN_HEIGHT/2 - y*SCREEN_HEIGHT/2));
}



void loop()
{
  while (1)  
  {
    if (x <= 360 )  
    {
      y[x] = sin(x*pi/180);  // argument degrees converted to radians
      GLCD.SetDot(xToScreen((float)x), yToScreen(y[x]), BLACK);
      x++;
    }
    else
    {
      GLCD.ClearScreen();
      x = 0;
    }
    
    delay(20);  //milliseconds
  }   
}
