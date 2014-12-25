#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <SD.h>
//#include <Wire.h>  
//#include <RTClib.h>
//#include <Time.h>
//#include <DS1307RTC.h>


/* 
 * a single revoluion of the wheel has this distance in mm (89,5 cm)
 */
#define distance 895 
#define interupt 3 // pin D1 on Leonardo!
#define chipSelect 10
//#define timeBetweenRounds 1800L // 30 minutes
#define timeBetweenRounds 300L // 5 minutes  
long curDistance = 0;  
#define LED  13

#define DEBUG 0

File dataFile;
File currentRunFile;
File runs;
long lastMeasurement = 0;
long beginRun = 0;

void setup()
{  
//  pinMode(LED, OUTPUT);

  attachInterrupt(interupt, BtnDownCB, FALLING );

  // Call the function to enable low power consumption
  #if !DEBUG
    ble_low_power();
  #else 
   Serial.begin(9600);
  #endif

  // If you are using the Blend, uncomment below
  ble_set_pins(6, 7);
   ble_set_name("Stroller");

  // Init. and start BLE library.
  ble_begin();

  // prepare SD data logging!
//
  if (!SD.begin(10, 11, 12, 13)) { // important: this is custom code for the leonardo, see https://learn.adafruit.com/adafruit-data-logger-shield/for-the-mega-and-leonardo

  } 
  else {

    // Open up the file we're going to log to!
    dataFile = SD.open("datalog.txt", FILE_WRITE);
     runs=SD.open("run.txt",FILE_WRITE);
  }


//  setSyncProvider(RTC.get);
}


void stroller_ble_print() {
  #if DEBUG
    Serial.println("printing distance");
  #endif
  if (ble_connected()) {
              char output[27];
            // length of output should be: 5 fixed, 10 time, 12 distance;
         sprintf(output,"D:%10lu:%10lu\n\n",millis(),curDistance);
          
          ble_write_bytes((unsigned char *)output,27);
  }
}

volatile int hadRound = 0;
void loop()
{
  long time = millis();

  if (hadRound== 1) {
        #if DEBUG
    Serial.println("had round");
  #endif
    // compare the time of the last logged or the current logged value
    // the actual calculation
      if (beginRun==0) {
        beginRun=time;
      } 
      if (lastMeasurement == 0 ){
        lastMeasurement = time;
      }
      curDistance = curDistance + distance;
//
//      // write to SD card

////      // here, we can write the full, readable timestamp:
              char output[27];
            // length of output should be: 5 fixed, 10 time, 12 distance;
            sprintf(output,"R:%10lu:%10.2lu\n",time,curDistance);
            dataFile.write(output,27);
            dataFile.flush();

      byte reset= 0;
      if ((lastMeasurement-time)>timeBetweenRounds) {
          reset=1;
          char result[100];
          sprintf(result,"Run:%10lu:%10lu:%10.2lf\n",beginRun,time,curDistance);
          runs.write(result,100);
        runs.flush();
        curDistance=distance;
        beginRun=millis();
      }
          stroller_ble_print();
          lastMeasurement = millis();
          hadRound=0;

  }
  if (ble_available() )
  {
    uint8_t temp;
    while ( ble_available() )
    {
      temp = ble_read();
      if ('r' == temp) { // read comand from smartphone)
          stroller_ble_print();
      } 
      else if ( 'c' == temp ) { // aCcept or commit command from smartphone, later used for resetting
        curDistance=0;
        ble_write('r');
        ble_write('\n');
      }
    }
  }
//  delay(50);
  digitalWrite(LED,LOW);
  ble_do_events();  
}

void BtnDownCB()
{   
  PRR0 = 0x00;  // Power Reduction Register: open timer
  PRR1 = 0x00;
  digitalWrite(LED,HIGH);
  hadRound = 1;
}



