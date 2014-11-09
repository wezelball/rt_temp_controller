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
float y1[360];       // this is the channel 1 y array
float y2[360];       // this is the channel 2 y array
int graphXMin = 20;  // the leftmost limit of the graph
int graphXMax = SCREEN_WIDTH-1;  // the rightmost limit of the graph
int graphYMin = 10;  // the top limit of the graph
int graphYMax = SCREEN_HEIGHT-1;  // the bottom limit of the graph
float ch1Setpoint = 0.5;  // ch.1 temperature setpoint
float ch2Setpoint = 0.5;  // ch.1 temperature setpoint

void setup()  {
  GLCD.Init(NON_INVERTED);     // initialise the library, non inverted writes pixels onto a clear screen
  GLCD.ClearScreen();
  GLCD.DrawRect(graphXMin,graphYMin,(graphXMax-graphXMin),(graphYMax-graphYMin),BLACK);
  GLCD.SelectFont(System5x7); // switch to fixed width system font
  Serial.begin(9600);  // open the serial port for debugging
}

/* Converts actual X value to screen coordinate
 * for plotting. Returns value between
 * graphXMin and graphXmax
 */
int xToScreen(float x, int screenXMin, int screenXMax) {
  return (int(x * (screenXMax-screenXMin)/360.0 + screenXMin));
}

/* Converts actual Y value to screen coordinate
 * for plotting. Returns value between
 * graphYMin and graphYMax
 */
int yToScreen(float y, int screenYMin, int screenYMax) {
  return(int(screenYMin + (screenYMax-screenYMin)/2 - y*(screenYMax-screenYMin)/2));
}

/*
 * Prints current information line temperature, output,
 * and setpoint to go at the top of the display
 */
void printCurrent() {
  
}

void loop()
{
  while (1)  
  {
    if (x <= 360 )  
    {
      y1[x] = sin(x*pi/180);  // argument degrees converted to radians
      y2[x] = cos(x*pi/180);
      GLCD.SetDot(xToScreen((float)x,graphXMin,graphXMax), yToScreen(y1[x],graphYMin,graphYMax), BLACK);
      GLCD.GotoXY(0, 0);
      GLCD.Puts("Ch1: ");
      GLCD.GotoXY(25,0);
      GLCD.print(y1[x],1);
      GLCD.GotoXY(50,0);
      GLCD.Puts("SP: ");
      GLCD.GotoXY(75,0);
      GLCD.PrintNumber(ch1Setpoint);
      GLCD.GotoXY(85,0);
      GLCD.Puts("Out: ");
      GLCD.GotoXY(110,0);
      GLCD.PrintNumber(85);
      x++;
    }
    else
    {
      GLCD.ClearScreen();
      x = 0;
      /* Draw the outer bounds of the graph area */
      GLCD.DrawRect(graphXMin,graphYMin,(graphXMax-graphXMin),(graphYMax-graphYMin),BLACK);
    }
    
    delay(200);  //milliseconds
  }   
 }
