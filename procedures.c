//#include <glcd.h>
#include <Arduino.h>

extern int LCDXMax;

/* This is equivalent to the map() function, except floats are allowed */
float convertRawPotValue(int value, float toMin, float toMax)
{
  float result;
  
  result = toMin + ((float)value/1023.0)*(toMax - toMin);
  return result;
}

/* Converts actual X value to screen coordinate
 * for plotting. Returns value between
 * LCDXMin and LCDXMax
 */
int xToScreen(int x, int screenXMin, int screenXMax) {
  if (x <= LCDXMax)
  {
    return (int)(x + screenXMin);
  }
  else
  {
    return (int)(LCDXMax);
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
