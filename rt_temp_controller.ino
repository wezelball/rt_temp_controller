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
float y[360];       // this is the y array
int graphXMin = 30;  // the leftmost limit of the graph
int graphXMax = 90;  // the rightmost limit of the graph
int graphYMin = 20;  // the top limit of the graph
int graphYMax = 50;  // the bottom limit of the graph

void setup()  {
  GLCD.Init(NON_INVERTED);     // initialise the library, non inverted writes pixels onto a clear screen
  GLCD.ClearScreen();
  GLCD.DrawBitmap(ArduinoIcon, 32,0, BLACK); //draw the bitmap at the given x,y position
  GLCD.ClearScreen();
  GLCD.DrawRect(graphXMin,graphYMin,(graphXMax-graphXMin),(graphYMax-graphYMin),BLACK);
  Serial.begin(9600);  // open the serial port for debugging
}

/* Converts actual X value to screen coordinate
 * for plotting. Returns value between
 * graphXMin and graphXmax
 */
int xToScreen(float x) {
  return (int(x * (graphXMax-graphXMin)/360.0 + graphXMin ));
}

/* Converts actual Y value to screen coordinate
 * for plotting. Returns value between
 * graphYMin and graphYMax
 */
int yToScreen(float y) {
  return(int(graphYMin + (graphYMax-graphYMin)/2 - y*(graphYMax-graphYMin)/2));
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
      /* Draw the outer bounds of the graph area */
      GLCD.DrawRect(graphXMin,graphYMin,(graphXMax-graphXMin),(graphYMax-graphYMin),BLACK);
    }
    
    delay(20);  //milliseconds
  }   
}
