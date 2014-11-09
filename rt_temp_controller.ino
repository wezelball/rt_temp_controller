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

int graphXMin = 20;  // the leftmost limit of the graph
int graphXMax = SCREEN_WIDTH-1;  // the rightmost limit of the graph
int graphYMin = 10;  // the top limit of the graph
int graphYMax = SCREEN_HEIGHT-1;  // the bottom limit of the graph
float ch1Setpoint[107];   // ch.1 temperature setpoint
float ch2Setpoint = 0.5;  // ch.1 temperature setpoint
float y1[107];       // this is the channel 1 y array
                     // graphXMax - graphXMin
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
  //return (int(x * (screenXMax-screenXMin)/360.0 + screenXMin));
  if (x <= graphXMax)
  {
    return (int(x + screenXMin));
  }
  else
  {
    return (int (graphXMax));
  }
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
void printCurrent(int channel, float value, int setpoint, int output) {
  GLCD.GotoXY(0, 0);
  GLCD.Puts("Ch: ");
  GLCD.GotoXY(20,0);
  GLCD.PrintNumber(channel);  // channel #
  GLCD.GotoXY(30,0);
  GLCD.print(value,1);        // temperature value
  GLCD.GotoXY(65,0);
  GLCD.Puts("S: ");
  GLCD.GotoXY(75,0);
  GLCD.PrintNumber(setpoint); //setpoint
  GLCD.GotoXY(95,0);
  GLCD.Puts("O: ");
  GLCD.GotoXY(110,0);
  GLCD.PrintNumber(output);   // PWM output
  // redraw rectangle
  GLCD.DrawRect(graphXMin,graphYMin,(graphXMax-graphXMin),(graphYMax-graphYMin),BLACK);
  
  // print the vertical axis numbers
  GLCD.GotoXY(4,graphYMin);
  GLCD.PrintNumber(1);
  GLCD.GotoXY(4,graphYMin -2 + (graphYMax-graphYMin)/2);
  GLCD.PrintNumber(0);
  GLCD.GotoXY(4,graphYMax-8);
  GLCD.PrintNumber(-1);
}

void loop()
{
  unsigned int i=0,j=0;
  
  while (1)  
  {
    // an equation with a little noise
    y1[i] = 0.0 + 0.002 * random(-100,100);  // argument degrees converted to radians
    ch1Setpoint[i]=0.0;
    
    // update the non-graph display values
    printCurrent(1,y1[i],30,80 );

    // the 106/107 is related to graphXMax-graphXMin
    if (i >= 106) 
    {
      for (j = 0; j < 107; j++)
      {
        // erase old dots
        GLCD.SetDot(xToScreen((float)j,graphXMin,graphXMax), yToScreen(y1[j],graphYMin,graphYMax), WHITE);
        GLCD.SetDot(xToScreen((float)j,graphXMin,graphXMax), yToScreen(ch1Setpoint[j],graphYMin,graphYMax), WHITE);
        // move values down by one
        y1[j] = y1[j+1];
        // redraw updated graph
        GLCD.SetDot(xToScreen((float)j,graphXMin,graphXMax), yToScreen(y1[j],graphYMin,graphYMax), BLACK);
        GLCD.SetDot(xToScreen((float)j,graphXMin,graphXMax), yToScreen(ch1Setpoint[j],graphYMin,graphYMax), BLACK);
      }
      // draw the new point
      GLCD.SetDot(xToScreen((float)i,graphXMin,graphXMax), yToScreen(y1[i],graphYMin,graphYMax), BLACK);
      GLCD.SetDot(xToScreen((float)i,graphXMin,graphXMax), yToScreen(ch1Setpoint[i],graphYMin,graphYMax), BLACK);
    }
    else
    {
      // this only executes until while the x values are less than graphXMax
      GLCD.SetDot(xToScreen((float)i,graphXMin,graphXMax), yToScreen(y1[i],graphYMin,graphYMax), BLACK);
      GLCD.SetDot(xToScreen((float)i,graphXMin,graphXMax), yToScreen(ch1Setpoint[i],graphYMin,graphYMax), BLACK);
      i++;
    }
    delay(1000);  // update time, in milliseconds
  }   
 }
