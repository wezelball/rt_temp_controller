//#include <glcd.h>
//#include <Arduino.h>


/* This is equivalent to the map() function, except floats are allowed */
float convertRawPotValue(int value, float toMin, float toMax)
{
  float result;
  
  result = toMin + ((float)value/1023.0)*(toMax - toMin);
  return result;
}

