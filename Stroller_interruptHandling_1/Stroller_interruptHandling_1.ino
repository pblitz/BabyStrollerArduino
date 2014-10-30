/*
Parts are:
Copyright (c) 2010, Donal Morrissey
All rights reserved.

*/

#include <avr/sleep.h>
#include <avr/power.h>

#define distance 20


long distanceToDate=0;
int pin2 = 2;
int hadRound=0;
void countRPM(void)
{
  detachInterrupt(0);
 hadRound=1;  

}


/***************************************************
 *  Name:        enterSleep
 *  Description: Enters the arduino into sleep mode.
 *
 ***************************************************/
void enterSleep(void)
{
  
  /* Setup pin2 as an interrupt and attach handler. */
  attachInterrupt(0, countRPM, LOW);
  delay(100);
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  sleep_enable();
  
  sleep_mode();
  
  /* The program will continue from here. */
  
  /* First thing to do is disable sleep. */
  sleep_disable(); 
}


/***************************************************
 *  Name:        setup
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Setup for the Arduino.           
 *
 ***************************************************/
void setup()
{
  Serial.begin(9600);
  
  /* Setup the pin direction. */
  pinMode(pin2, INPUT);
   
  /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);
  
  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  
  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE);
  
  Serial.println("Initialisation complete.");
  delay(100);
}


volatile int f_wdt=1;



/***************************************************
 *  Name:        ISR(WDT_vect)
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Watchdog Interrupt Service. This
 *               is executed when watchdog timed out.
 *
 ***************************************************/
ISR(WDT_vect)
{
  Serial.print(millis());
  Serial.println(" Watchdog overrun");
}



/***************************************************
 *  Name:        loop
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Main application loop.
 *
 ***************************************************/
int seconds=0;
void loop()
{
  if (hadRound==1) {
   distanceToDate=distanceToDate+distance;
  Serial.print("Distance is now: ");
  Serial.print(distanceToDate);
  Serial.print(" at  ");
  Serial.println(millis());
  hadRound=0;
  delay(100);
  } 
    delay(100);
      Serial.print(millis());

      Serial.println(" going back to sleep");
    enterSleep();
}
