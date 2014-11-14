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

/* Includes */ 
#include <glcd.h>
#include "fonts/Arial14.h"        // proportional font
#include "fonts/SystemFont5x7.h"  // system font
#include "bitmaps/ArduinoIcon.h"  // bitmap
#include "procedures.c"           // my procedures

/* constants (thou art not evil) */
const float pi = 3.1416;          // good ole pi
const int SCREEN_WIDTH = 128;     // KS0108 LCD max screen width x
const int SCREEN_HEIGHT = 64;     // KS0108 LCD max screen height y

/* global variables (thou art evil) */
unsigned int channelSelected = 1;// can display 2 channels
unsigned int i=0,j=0;            // loop counter variables

/* The LCD coordinates are the limits of the plotting area */
int LCDXMin = 25;              // the leftmost limit of the graph, in LCD coordinates
int LCDXMax = SCREEN_WIDTH-1;  // the rightmost limit of the graph, in LCD coordinates
int LCDYMin = 10;              // the top limit of the graph, in LCD coordinates
int LCDYMax = SCREEN_HEIGHT-1; // the bottom limit of the graph, in LCD coordinates

/* The limits of the y actual data */
//float y1Min = -1.0;
float y1Min;
//float y1Max = 1.0;
float y1Max;
//float y2Min = 0.0;
float y2Min;
//float y2Max = 10.0;
float y2Max;

/* The plot center and window values */
float y1Center = 20;
float y1Window = 10;
float y2Center = 30;
float y2Window = 5;

float ch1Setpoint[107];          // ch.1 temperature setpoint
float ch2Setpoint[107];          // ch.2 temperature setpoint
float y1[107];                   // this is the channel 1 y array
float y2[107];                   // LCDXMax - LCDXMin

/* Inputs/Outputs */
int potRawInput = A0;
int ch1Switch = A2;
int ch2Switch = A1;
    
void setup()  {
  // GLCD
  GLCD.Init(NON_INVERTED);       // initialise the library, non inverted writes pixels onto a clear screen
  GLCD.ClearScreen();
  GLCD.DrawRect(LCDXMin,LCDYMin,(LCDXMax-LCDXMin),(LCDYMax-LCDYMin),BLACK);
  GLCD.SelectFont(System5x7);   // switch to fixed width system font
  // Serial for debugging
  Serial.begin(9600);           // open the serial port for debugging
  //  Physical I/O
  pinMode(ch1Switch, INPUT_PULLUP);
  pinMode(ch2Switch, INPUT_PULLUP);
  
  // setup the initial y min and max plot ranges
  y1Min = y1Center - y1Window/2;
  y1Max = y1Center + y1Window/2;
  y2Min = y2Center - y2Window/2;
  y2Max = y2Center + y2Window/2;
}

/* Converts actual X value to screen coordinate
 * for plotting. Returns value between
 * LCDXMin and LCDXMax
 */
int xToScreen(int x, int screenXMin, int screenXMax) {
  //return (int(x * (screenXMax-screenXMin)/360.0 + screenXMin));
  if (x <= LCDXMax)
  {
    return (int(x + screenXMin));
  }
  else
  {
    return (int (LCDXMax));
  }
}

/* Converts actual Y value to screen coordinate
 * for plotting. Returns value between
 * LCDYMin and LCDYMax
 */
int yToScreen(float y, int screenYMin, int screenYMax, float yMin, float yMax) {
  int screenY;
  
  screenY = screenYMax + ((y - yMin) * (screenYMin - screenYMax))/(yMax - yMin);
  return screenY;
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
  GLCD.GotoXY(60,0);
  GLCD.Puts("S: ");
  GLCD.GotoXY(70,0);
  potRawInput = analogRead(A0);
  GLCD.print(convertRawPotValue(potRawInput, 0, 10),2); //setpoint
  GLCD.GotoXY(96,0);
  GLCD.Puts("O: ");
  GLCD.GotoXY(110,0);
  GLCD.PrintNumber(output);   // PWM output
  // redraw rectangle
  GLCD.DrawRect(LCDXMin,LCDYMin,(LCDXMax-LCDXMin),(LCDYMax-LCDYMin),BLACK);
  
  // print the vertical axis numbers
  if (channel == 1)
  {
    GLCD.GotoXY(1,LCDYMin);
    GLCD.print(y1Max,1);
    GLCD.GotoXY(1,LCDYMin -4 + (LCDYMax-LCDYMin)/2);
    GLCD.print(y1Min+(y1Max-y1Min)/2,1);
    GLCD.GotoXY(1,LCDYMax-8);
    GLCD.print(y1Min,1);
  }
  else
  {
    GLCD.GotoXY(1,LCDYMin);
    GLCD.print(y2Max,1);
    GLCD.GotoXY(1,LCDYMin -4 + (LCDYMax-LCDYMin)/2);
    GLCD.print(y2Min+(y2Max-y2Min)/2,1);
    GLCD.GotoXY(1,LCDYMax-8);
    GLCD.print(y2Min,1);
  }
}

/* Erase dot on the screen by plottiing in background color */ 
void eraseDot(int x, float y, float yMin, float yMax)
{
  GLCD.SetDot(xToScreen(x,LCDXMin,LCDXMax), yToScreen(y,LCDYMin,LCDYMax,yMin,yMax), WHITE);
}

/* Plot dot on the screen */ 
void drawDot(int x, float y, float yMin, float yMax)
{
  GLCD.SetDot(xToScreen(x,LCDXMin,LCDXMax), yToScreen(y,LCDYMin,LCDYMax,yMin,yMax), BLACK);
}

void loop()
{
  /* Handle channel selection by flpping channel switch */
  if (digitalRead(ch1Switch) == HIGH)
    {
      // This is true only when switching from opposite channel
      if (channelSelected != 1)
      {
        channelSelected = 1;
        i = 0;
        j = 0;
        GLCD.ClearScreen();
      }  
    }
    if (digitalRead(ch2Switch) == HIGH)
    {
      // This is true only when switching from opposite channel
      if (channelSelected != 2)
      {
        channelSelected = 2;
        i = 0;
        j = 0;
        GLCD.ClearScreen();
      }
    }

      
    // an equation with a little noise
    y1[i] = 21.0 + 0.002 * random(-100,100);  // simulated noisy temperature
    ch1Setpoint[i]=0.0;
    y2[i] = 32.0 + 0.004 * random(-100,100);  // simulated noisy temperature
    ch2Setpoint[i]=0.5;
    
    // update the axis and graph header values
    if (channelSelected == 1) 
      printCurrent(1,y1[i],30,80 );
    else
      printCurrent(2,y1[i],30,80 );

    // We are now plotting past end of graph, so 
    // we need to scroll
    if (i >= (LCDXMax-LCDXMin-1))
    {
      for (j = 0; j < (LCDXMax-LCDXMin); j++)
      {
        if (channelSelected == 1)
        {
          // erase old dots
          eraseDot(j,y1[j],y1Min,y1Max);
          eraseDot(j,ch1Setpoint[j],y1Min,y1Max);
        
          // move values down by one
          y1[j] = y1[j+1];
        
          // redraw updated graph
          drawDot(j,y1[j],y1Min,y1Max);
          drawDot(j,ch1Setpoint[j],y1Min,y1Max);
        }
        else
        {
          // erase old dots
          eraseDot(j,y2[j],y2Min,y2Max);
          eraseDot(j,ch2Setpoint[j],y2Min,y2Max);
        
          // move values down by one
          y2[j] = y2[j+1];
        
          // redraw updated graph
          drawDot(j,y2[j],y2Min,y2Max);
          drawDot(j,ch2Setpoint[j],y2Min,y2Max);
        }
      }
      // draw the new point
      if (channelSelected == 1)  // channel 1 plot
      {
        drawDot(i,y1[i],y1Min,y1Max);
        drawDot(i,ch1Setpoint[i],y1Min,y1Max);
      }
      else
      {
        drawDot(i,y2[i],y2Min,y2Max);        // channel 2 plot
        drawDot(i,ch2Setpoint[i],y2Min,y2Max);
      }
    }
    else
    {
      // this only executes until while the x values are less than LCDXMax
      if (channelSelected == 1)
      {
        drawDot(i,y1[i],y1Min,y1Max);
        drawDot(i,ch1Setpoint[i],y1Min,y1Max);
      }
      else
      {
        drawDot(i,y2[i],y2Min,y2Max);
        drawDot(i,ch2Setpoint[i],y2Min,y2Max);
      }
      
      i++;
    }
    delay(100);  // update time, in milliseconds
}
