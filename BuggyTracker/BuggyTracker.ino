#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>

  /* 
  * a single revoluion of the wheel has this distance in mm (89,5 cm)
   */
  #define distance 895 

long curDistance = 0;
int LED = 13;
const int interupt = 3; // pin D1 on Leonardo!

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
}

volatile int hadRound = 0;

void loop()
{
  if (hadRound== 1) {
     // compare the time of the last logged or the current logged value
     // the actual calculation
     curDistance = curDistance + distance;
     
     hadRound=0;
     
  }
    if (ble_available() )
    {
        uint8_t temp;
        while ( ble_available() )
        {
          temp = ble_read();
          if ('r' == temp) { // read comand from smartphone)
            String output = "D: ";
            output+=curDistance;
            output+="\n";
            int length= output.length()+1;
            char charBuf[length];
            output.toCharArray(charBuf,output.length()+1);
            for (int i=0; i<length;i++) {
              ble_write(charBuf[i]);
            } 
          } else if ( 'c' == temp ) { // aCcept or commit command from smartphone, later used for resetting
            ble_write_bytes((unsigned char *)"not yet implemented", 19);
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
