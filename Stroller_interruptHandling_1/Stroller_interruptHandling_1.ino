  /*
  Parts are:
  Copyright (c) 2010, Donal Morrissey
  All rights reserved.
  
  */
  
  /*
  Includes for Power Management
  */
  
  #include <avr/sleep.h>
  #include <avr/power.h>
  /*
  Includes for Blend BLE functions
  */
  #include <SPI.h>
  #include <boards.h>
  #include <RBL_nRF8001.h>
  /* 
  * a single revoluion of the wheel has this distance in mm (89,5 cm)
   */
  #define distance 895 
  /*
   * if true, print serial output
   */
  int debug = 1;
  
  long distanceToDate=0;
  int pinOut = 13;
  int pin2 = 3;
  volatile  int hadRound=LOW;
  int inWakeLock=0;
  int ble_enabled = 0;
  const int ledPin =  12; 
  volatile long beginBLETime=0;
  int countTimesSend=0;
  
  void countRPM(void)
  {
        Serial.println("In count");
  
    sleep_disable(); 
    detachInterrupt(0);
    hadRound=1;
  
  }
  
  void enableBLE(void) {
    Serial.println("Enabling BLE");
    digitalWrite(pinOut,HIGH);
    //    ble_low_power();
    ble_set_pins(6, 7);
  
    // Set your BLE Shield name here, max. length 10
    ble_set_name("Stroller");
    // Init. and start BLE library.
    ble_begin();
    beginBLETime=millis();
  }
  
  
  /***************************************************
   *  Name:        enterSleep
   *  Description: Enters the arduino into sleep mode.
   *
   ***************************************************/
  void enterSleep(void)
  {
  //      Serial.println("Going to sleep");
    
    /* Setup pin2 as an interrupt and attach handler. */
     if (digitalRead(2)==HIGH){
    sleep_enable();
  
    attachInterrupt(0, countRPM, FALLING);
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //        set_sleep_mode(SLEEP_MODE_IDLE);
          Serial.println("Sleep mode is idle");
  //  delay(100);
  //  cli();
  //  sleep_bod_disable();
  //sei();
    
  
    
    
    sleep_cpu();
    
    /* The program will continue from here. */
    
    /* First thing to do is disable sleep. */
    sleep_disable(); 
     } else {
  //     Serial.println("not yet going to sleep"); 
     }
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
    pinMode(pinOut, OUTPUT);
    pinMode(ledPin, OUTPUT);  
    // Disabled the watchdog service for now
    /* Clear the reset flag. */
  //  MCUSR &= ~(1<<WDRF);
    
    /* In order to change WDE or the prescaler, we need to
     * set WDCE (This will allow updates for 4 clock cycles).
     */
  //  WDTCSR |= (1<<WDCE) | (1<<WDE);
  
    /* set new watchdog timeout prescaler value */
  //  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
    
    /* Enable the WD interrupt (note no reset). */
  //  WDTCSR |= _BV(WDIE);
  // 
    
    Serial.println("Initialisation complete.");

      attachInterrupt(0, countRPM, FALLING);
      attachInterrupt(1, enableBLE, FALLING);
  //  delay(100);
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
  //  Serial.print(millis());
  //  Serial.println(" Watchdog overrun");
  //  Serial.println(" ");
  //  Serial.print("Distance: ");
  //  Serial.println(distanceToDate);
    inWakeLock=HIGH;
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
    digitalWrite(ledPin,HIGH);
    if (hadRound==HIGH) {
      distanceToDate=distanceToDate+distance;
      digitalWrite(pinOut,HIGH);
      if (debug==HIGH) {
        Serial.print("Distance is now: ");
        Serial.print(distanceToDate);
        Serial.print(" at  ");
        Serial.println(millis());
      }
      hadRound=0;
      delay(100);
    } 
   digitalWrite(pinOut,LOW);
   if (debug==HIGH) {
      Serial.print(millis());
      Serial.println("going back to sleep");
      delay(50);
   }
  
//   if (ble_available() )
//   {
//         // write to BLE (even if not connected)
//        String output = "D: ";
//        output+=distanceToDate;
//        output+="\n";
//        int length= output.length()+1;
//        char charBuf[length];
//        output.toCharArray(charBuf,output.length()+1);
//        for (int i=0; i<length;i++) {
//          ble_write(charBuf[i]);
//        } 
//    }
//    ble_do_events();  
    delay(100);
    if (beginBLETime>0) {
      // we don't go into sleep after one minute of pressing the BLE button!    
      long runningMillis=millis()-beginBLETime;
      
      if (ble_available() )
      {
         // write to BLE (even if not connected)
        String output = "D: ";
        output+=distanceToDate;
        output+="\n";
        int length= output.length()+1;
        char charBuf[length];
        output.toCharArray(charBuf,output.length()+1);
        for (int i=0; i<length;i++) {
          ble_write(charBuf[i]);
        } 
  
    }
    if ( runningMillis>20000) {
       // in the next iteration, stop this maddness and go back to sleep
       beginBLETime=0;
       countTimesSend=0;
       
    }
     attachInterrupt(0, countRPM, FALLING);

    ble_do_events();  
          digitalWrite(ledPin,LOW);

    } else {
          digitalWrite(ledPin,LOW);

      enterSleep();
    }
  }
