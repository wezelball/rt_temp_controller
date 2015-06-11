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
// OpenGLCD
#include <openGLCD_Config.h>
#include <openGLCD_Buildinfo.h>
#include <openGLCD.h>
#include "fonts/Arial14.h"        // proportional font
#include "fonts/SystemFont5x7.h"  // system font
#include "bitmaps/ArduinoIcon.h"  // bitmap
// Dallas 1-wire
#include <OneWire.h>
// PID library
#include <PID_v1.h>
// My procedures
#include "procedures.c"           // my procedures

// Identfiers for temperature controllers
#define DS18S20_ID 0x10
#define DS18B20_ID 0x28

/* constants (thou art not evil) */
const float pi = 3.1416;          // good ole pi
const int SCREEN_WIDTH = 128;     // KS0108 LCD max screen width x
const int SCREEN_HEIGHT = 64;     // KS0108 LCD max screen height y
const int npn_ch1 = 2;            // The npn transistor is on digital pin 2


/* global variables (thou art evil) */
unsigned int channelSelected = 1;// can display 2 channels
unsigned int i=0,j=0;            // loop counter variables

/* The LCD coordinates are the limits of the plotting area */
int LCDXMin = 25;              // the leftmost limit of the graph, in LCD coordinates
int LCDXMax = SCREEN_WIDTH-1;  // the rightmost limit of the graph, in LCD coordinates
int LCDYMin = 10;              // the top limit of the graph, in LCD coordinates
int LCDYMax = SCREEN_HEIGHT-1; // the bottom limit of the graph, in LCD coordinates

/* The limits of the y actual data */
float y1Min;
float y1Max;
float y2Min;
float y2Max;

/* The plot center and window values */
float y1Center = 20;
float y1OldCenter = 20;
float y1Window = 2;
float y2Center = 30;
float y2OldCenter = 30;
float y2Window = 5;

float ch1PotOffset;	// the current potentiometer offset from initial
float ch1PotInitial;	// the inital pot value when switch thrown to ch1
float ch1PotSetpoint;

/* Need to know when switch first thrown to position 
 * 
 * variable assumes value of 0, 1, or 2 based on last switch position
 * 
 */
int lastSwitchState;
float ch1Setpoint[107];         // ch.1 temperature setpoint
float ch2Setpoint[107];         // ch.2 temperature setpoint
float y1[107];                  // this is the channel 1 y array
float y2[107];                  // LCDXMax - LCDXMin
double Setpoint, Input, Output; // for the temperature PID
double Kp=20.0;                 // proportional gain
double Ki=1.50;                  // integral gain
double Kd=0.1;                  // derivative gain
double Bias=0.0;                // feed-forward
float error;                    // the difference between setpoint and feedback
float last_temperature;
int npn_ch1_level = 0;  //This is a PWM value from 0 to 255 that actually controls the transistor

struct ds1820_temperature {
  boolean dataValid;
  float degreesC;
};

// Specify the links and initial tuning parameters
// Feedback, Process variable, Setpoint, Kp, Ki, Kd, Direction
PID lightbulbPID(&Input, &Output, &Setpoint, Kp,Ki,Kd,DIRECT);

/* Inputs/Outputs */
int potRawInput = A0;
int ch1Switch = A2;
int ch2Switch = A1;
OneWire ds(A3);                  // onwire DS18B20 newtork on A3

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

  // determine the initial switch value
  if (digitalRead(ch1Switch) == HIGH)
    lastSwitchState = 1;
  else if (digitalRead(ch2Switch) == HIGH)
    lastSwitchState = 2;
  else
    lastSwitchState = 0;
  
  // Turn on the PID
  lightbulbPID.SetMode(AUTOMATIC);  
}

/*
 * Prints current information line temperature, output,
 * and setpoint to go at the top of the display
 */
void printCurrent(int channel, float value, double setpoint, int output) {
  GLCD.GotoXY(0, 0);
  GLCD.Puts("Ch: ");
  GLCD.GotoXY(20,0);
  GLCD.PrintNumber(channel);  // channel #
  GLCD.GotoXY(30,0);
  GLCD.print(value,1);        // temperature value
  GLCD.GotoXY(60,0);
  GLCD.Puts("S: ");           // setpoint
  GLCD.GotoXY(70,0);
  //potRawInput = analogRead(A0);
  //GLCD.print(convertRawPotValue(potRawInput, 0, 10),2); //setpoint
  GLCD.print(setpoint,1); //setpoint
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
	if (y >= yMin && y <= yMax)	// only if y data in graph boundary
		GLCD.SetDot(xToScreen(x,LCDXMin,LCDXMax), yToScreen(y,LCDYMin,LCDYMax,yMin,yMax), WHITE);
}

/* Plot dot on the screen */
void drawDot(int x, float y, float yMin, float yMax)
{
	if (y >= yMin && y <= yMax)	// only if y data in graph boundary
		GLCD.SetDot(xToScreen(x,LCDXMin,LCDXMax), yToScreen(y,LCDYMin,LCDYMax,yMin,yMax), BLACK);
}

struct ds1820_temperature getTemperature(){
  struct ds1820_temperature temp;
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  //find a device
  if (!ds.search(addr)) {
    ds.reset_search();
    temp.dataValid=false;
    // Note
    // This search fails every other iteration
    // I am properly handling it, but
    // fix would be nice
    //Serial.println("can't find device");
    return temp;
  }
  if (OneWire::crc8( addr, 7) != addr[7]) {
    temp.dataValid=false;
    //Serial.println("CRC failure");
    return temp;
  }
  if (addr[0] != DS18S20_ID && addr[0] != DS18B20_ID) {
    temp.dataValid=false;
    //Serial.println("can't identify device");
    return temp;
  }
  ds.reset();
  ds.select(addr);
  // Start conversion
  ds.write(0x44, 1);
  // Wait some time...
  delay(850);
  present = ds.reset();
  ds.select(addr);
  // Issue Read scratchpad command
  ds.write(0xBE);
  // Receive 9 bytes
  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  // Calculate temperature value
  temp.degreesC = ( (data[1] << 8) + data[0] )*0.0625;
  temp.dataValid = true;
  return temp;
}

void loop()
{

  struct ds1820_temperature temperature = getTemperature();

  if (temperature.dataValid) {
		if (temperature.degreesC > 0.0 && temperature.degreesC < 70.0) {
			Input = (double)temperature.degreesC;
			last_temperature = temperature.degreesC; 
		}
		else {
			Input = (double)last_temperature;
		}
	} 
  else {
    Input = (double)last_temperature;
  }

  Serial.print(Setpoint);
  Serial.print("\t");
  Serial.print(Input);
  Serial.print("\t");
  Serial.println(npn_ch1_level);

  // Execute the PID algrithm
  lightbulbPID.Compute();
  // Output disabled until I can figure out problem with sensor
  //npn_ch1_level = int(Output) + Bias;
  error = Input - Setpoint;
  
  // Write to the external driver circuitry
  // Output disabled until I can figure out problem with sensor
  //analogWrite(npn_ch1, npn_ch1_level); //Write this new value out to the port

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
      Setpoint = Input;   // match setpoint to current temperature plus a margin
      y1Center = (float)Input;	// center the display on current temp
      ch1PotOffset = 0.0;
      ch1PotInitial = convertRawPotValue(analogRead(A0), 0, 10);
      ch1PotSetpoint = (float)Setpoint;
    }
    // True if switching from any other switch position
    // like a rising-edge detector
    if (lastSwitchState != 1)
    {
      lastSwitchState = 1;
      GLCD.ClearScreen();
      Setpoint = Input;   // match setpoint to current temperature plus a margin
      y1Center = (float)Input;	// center the display on current temp
      ch1PotOffset = 0.0;
      ch1PotInitial = convertRawPotValue(analogRead(A0), 0, 10);
      ch1PotSetpoint = (float)Setpoint;
    }
    else  // allow temperature window adjustment with the pot
    {
		// Read the potentiometer  and calculate setpoint
		ch1PotOffset = (convertRawPotValue(analogRead(A0), 0, 10) - ch1PotInitial);
		Setpoint  = ch1PotSetpoint + ch1PotOffset;
		
		// Recalculate the screen limits
      y1Min = y1Center - y1Window/2;
      y1Max = y1Center + y1Window/2;
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
    // True if switching from any other switch position
    // like a rising-edge detector
    if (lastSwitchState != 2)
    {
      lastSwitchState = 2;
    }
    else  // allow temperature window adjustment with the pot
    {
      y2Center = y2OldCenter + (convertRawPotValue(analogRead(A0), 0, 40)); 
      y2Center = convertRawPotValue(analogRead(A0), 0, 40);
      y2Min = y2Center - y2Window/2;
      y2Max = y2Center + y2Window/2;
      GLCD.ClearScreen();
    }
  }

	// Update last switch state if switch in center position
	if (digitalRead(ch1Switch) == LOW && digitalRead(ch2Switch) == LOW)
	lastSwitchState = 0;  

	y1[i] = Input;	// this is a real temperature
	ch1Setpoint[i] = Setpoint;	// channel 1 setpoint
  
	// an equation with a little noise
	y2[i] = 32.0 + 0.004 * random(-100,100);  // simulated noisy temperature
	ch2Setpoint[i]=32.0;

	// update the axis and graph header values
	if (channelSelected == 1) 
		printCurrent(1,y1[i],Setpoint,npn_ch1_level );
	else
		printCurrent(2,y2[i],30,80 );

	/* Below this is where the temperature graph is being drawn on screen */

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

