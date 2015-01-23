#ifndef EXC_time
  #define EXC_time


/******************************************************************/
//  FUNCIONES RELOJ
/*****************************************************************/
#if defined (moduleDS1307) || defined (moduleDS3231)
// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );

}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );

}

// Stops the DS1307, but it has the side effect of setting seconds to 0
// Probably only want to use this for testing
void startDs1307()
{
  Wire.beginTransmission(DS_RTC);  
  Wire.write(0);
  Wire.write(0x00);
  Wire.endTransmission();
}


// 1) Sets the date and time on the ds1307
// 2) Starts the clock
// 3) Sets hour mode to 24 hour clock
// Assumes you're passing in valid numbers
void setDateDs1307()/*byte second,        // 0-59
                   byte minute,        // 0-59
                   byte hour,          // 1-23
                   byte dayOfWeek,     // 1-7
                   byte dayOfMonth,    // 1-28/29/30/31
                   byte month,         // 1-12
                   byte year)          // 0-99*/
{
   Wire.beginTransmission(DS_RTC);
   Wire.write(0);
   Wire.write(decToBcd(second));    // 0 to bit 7 starts the clock
   Wire.write(decToBcd(minute));
   Wire.write(decToBcd(hour));      // If you want 12 hour am/pm you need to set
                                   // bit 6 (also need to change readDateDs1307)
   Wire.write(decToBcd(dayOfWeek));
   Wire.write(decToBcd(dayOfMonth));
   Wire.write(decToBcd(month));
   Wire.write(decToBcd(year));
   Wire.endTransmission();
}

// Gets the date and time from the ds1307
void getDateDs1307()
{
  // Reset the register pointer
  Wire.beginTransmission(DS_RTC);
  Wire.write(0);
  Wire.endTransmission();
 
  Wire.requestFrom(DS_RTC, 7);

  // A few of these need masks because certain bits are control bits
  if (Wire.available()==7){
    second     = bcdToDec(Wire.read() & 0x7f);
    minute     = bcdToDec(Wire.read());
    hour       = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
    dayOfWeek  = bcdToDec(Wire.read());
    dayOfMonth = bcdToDec(Wire.read());
    month      = bcdToDec(Wire.read());
    year       = bcdToDec(Wire.read());
  }
}
#else
  void setDateDs1307(){}:
  void getDateDS1307(){}:

#endif

#endif

