#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <SD.h>
#include <Wire.h>  
#include <RTClib.h>
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

File dataFile;
File currentRunFile;
File runs;
RTC_DS1307 RTC;

void setup()
{  
//  pinMode(LED, OUTPUT);

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
    currentRunFile = SD.open("currun.txt", FILE_WRITE);
     runs=SD.open("run.txt",FILE_WRITE);
  }
    Wire.begin();
    RTC.begin();
//  setSyncProvider(RTC.get);
}


volatile int hadRound = 0;


void loop()
{
        union {
        char data[4];
        long stamp;
      } time;
        time.stamp = RTC.now().unixtime();
      char chrTime[10];
      ltoa(time.stamp,chrTime,10);
  if (hadRound== 1) {
    // compare the time of the last logged or the current logged value
    // the actual calculation

      curDistance = curDistance + distance;

      // write to SD card

      // here, we can write the full, readable timestamp:
      dataFile.write(chrTime, 10);
      dataFile.write(';');
      dataFile.write('R');
      dataFile.write('\n');
      dataFile.flush();
      currentRunFile.seek(0); // always start at the beginning of the file
      // read the first four bytes of the file, that contain the last written timestamp
      
      union{
        char data[4];
        long stamp;
      } lastTime ;
      //read the last saved timestamp
       lastTime.data[0]=currentRunFile.read();
       lastTime.data[1]=currentRunFile.read();
       lastTime.data[2]=currentRunFile.read();
       lastTime.data[3]=currentRunFile.read();
       
      byte reset= 0;
      if ((time.stamp-lastTime.stamp)>timeBetweenRounds) {
        // we have a new file, reset the distance and save to a new file

        char distanceBuf[10];
        ltoa(curDistance,distanceBuf,10);
                int length;

   for (byte i = 0;i< sizeof(distanceBuf);i++){
       if (distanceBuf[i]==NULL) {
              length=i+1;
              break;
            } 
         }
         char outBuf[length];
         for (byte i = 0;i<length;i++){
            outBuf[i]=distanceBuf[i];
         } 
        runs.write(chrTime, 10);
        runs.write(':');
        runs.write(outBuf, length);
        reset=1;
        currentRunFile.read(); //";"
//        currentRunFile.read();//"Distance" 1
//        currentRunFile.read();//"Distance" 2
//        currentRunFile.read();//"Distance" 3
//        currentRunFile.read();//"Distance" 4
//        currentRunFile.read();//";"

        union{
          char data[4];
          long stamp;
        } startingTime ;
        
       startingTime.data[0]=currentRunFile.read();
       startingTime.data[1]=currentRunFile.read();
       startingTime.data[2]=currentRunFile.read();
         startingTime.data[3]=currentRunFile.read();
        char startingTimeOut[10];
        ltoa(startingTime.stamp,startingTimeOut,10);
        runs.write(':');
        runs.write(startingTimeOut, 10);
        runs.write('\n');
        runs.flush();
        curDistance=distance;
      } 
      currentRunFile.seek(0);
      currentRunFile.write(time.data, 4);
//      currentRunFile.write(':');
//      currentRunFile.write((unsigned char*)&curDistance, 4);      
      if (reset==1) {
        currentRunFile.write(':');
        currentRunFile.write(time.data, 4); // origin time, will not be changed later on
        currentRunFile.flush();
      }      
      
      
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
//        String output = "D:";
//        output+=curDistance;
//        output+="\n";
//        byte length= 8;
          
        
        char distanceBuf[10];
        
        ltoa(curDistance,distanceBuf,10);
        int length;

   for (byte i = 0;i<sizeof(distanceBuf);i++){
       if (distanceBuf[i]==NULL) {
              length=i+1;
              break;
            } 
         }
         char outBuf[length];
         for (byte i = 0;i<length;i++){
            outBuf[i]=distanceBuf[i];
         } 
         


//         ble_write(length);
         ble_write('D');
         ble_write(':');
          ble_write_bytes((unsigned char *)outBuf,length);
          ble_write('\n');
//          ble_write_bytes();
      } 
      else if ( 'c' == temp ) { // aCcept or commit command from smartphone, later used for resetting
        curDistance=0;
        ble_write('r');
        ble_write('\n');
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
//  digitalWrite(LED,LOW);
  ble_do_events();  
}

void BtnDownCB()
{   
  PRR0 = 0x00;  // Power Reduction Register: open timer
  PRR1 = 0x00;
//  digitalWrite(LED,HIGH);
  hadRound = 1;
  //    delay(100);  //jitters elimination
  //    if(LOW == digitalRead(1))
//  if ( ble_connected() )
//  {
//    ble_write_bytes((unsigned char *)"Button down", 11);
//  }

}



