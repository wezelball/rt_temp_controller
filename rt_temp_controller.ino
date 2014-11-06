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


/* global variables (thou art evil) */
unsigned int x = 0;
float y = 0.0;
float pi = 3.1416;

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
  return (int(x * (128.0/360.0)));
}

int yToScreen(float y) {
  return(int(32 - y*32));
}

void loop()
{
  while (1)  
  {
    if (x <= 360 )  
    {
      y=sin(x*pi/180);  // argument degrees converted to radians
      GLCD.SetDot(xToScreen((float)x), yToScreen(y), BLACK);
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
