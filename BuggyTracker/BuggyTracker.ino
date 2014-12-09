#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <SD.h>
#include <Wire.h>  
#include <RTClib.h>
//#include <DS1307RTC.h>


/* 
 * a single revoluion of the wheel has this distance in mm (89,5 cm)
 */
#define distance 895 

long curDistance = 0;
int LED = 13;
const int interupt = 3; // pin D1 on Leonardo!
const int chipSelect = 10;
File dataFile;
RTC_DS1307 RTC;

void setup()
{  
  pinMode(LED, OUTPUT);

  attachInterrupt(interupt, BtnDownCB, FALLING );

  // Call the function to enable low power consumption
  ble_low_power();

  // If you are using the Blend, uncomment below
  ble_set_pins(6, 7);
  ble_set_name("Stroller");

  // Init. and start BLE library.
  ble_begin();

  // prepare SD data logging!

  if (!SD.begin(10, 11, 12, 13)) { // important: this is custom code for the leonardo, see https://learn.adafruit.com/adafruit-data-logger-shield/for-the-mega-and-leonardo
//
  } 
  else {

    // Open up the file we're going to log to!
    dataFile = SD.open("datalog.txt", FILE_WRITE);

  }

}


volatile int hadRound = 0;


void loop()
{
  if (hadRound== 1) {
    // compare the time of the last logged or the current logged value
    // the actual calculation
      curDistance = curDistance + distance;
      // write to SD card
      long time = RTC.now().unixtime();
    
  
    
      hadRound=0;

  }
  if (ble_available() )
  {
    uint8_t temp;
    while ( ble_available() )
    {
      temp = ble_read();
      if ('r' == temp) { // read comand from smartphone)
//        int length = 4+sizeof(curDistance);
        String output = "D:";
        output+=curDistance;
        output+="\n";
        byte length= output.length()+1;

        unsigned char charBuf[length];
        charBuf[0]='D';
        charBuf[1]=':';
          ble_write('D');
          ble_write(':');
//           byte b[4];
//           ble_write((int)curDistance>>24);
//           ble_write((int)curDistance>>16);
//           ble_write((int)curDistance>>8);
//           ble_write((int)curDistance);
//          IntegerToBytes(curDistance, b);
          for (byte i=0; i<length; ++i) {
              ble_write(charBuf[i]);

          }
          ble_write('\n');
      } 
      else if ( 'c' == temp ) { // aCcept or commit command from smartphone, later used for resetting
        curDistance=0;
        ble_write_bytes((unsigned char *)"reset", 5);
      }
      //            temp = ble_read();
      //            if('o' == temp)
      //            {
      //                temp = ble_read();
      //                if('n' == temp)
      //                {
      //
      //                    ble_write_bytes((unsigned char *)"LED on", 6);
      //                }
      //                else if('f' == temp && 'f' == ble_read())
      //                {
      //                    ble_write_bytes((unsigned char *)"LED off", 7);
      //                }
      //            }
    }
  }
  //    delay(100);
  digitalWrite(LED,LOW);
  ble_do_events();  
}

void BtnDownCB()
{   
  PRR0 = 0x00;  // Power Reduction Register: open timer
  PRR1 = 0x00;
  digitalWrite(LED,HIGH);
  hadRound = 1;
  //    delay(100);  //jitters elimination
  //    if(LOW == digitalRead(1))
  if ( ble_connected() )
  {
    ble_write_bytes((unsigned char *)"Button down", 11);
  }

}

