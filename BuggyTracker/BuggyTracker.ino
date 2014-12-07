#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>

int LED = 13;
int Button = 1;
const int interupt = 3; // pin D1 on Leonardo!

void setup()
{  
    pinMode(LED, OUTPUT);

    pinMode(Button, INPUT);
    pinMode(12,OUTPUT);
    digitalWrite(Button, HIGH);
    attachInterrupt(interupt, BtnDownCB, FALLING );

    // Call the function to enable low power consumption
    ble_low_power();

    // If you are using the Blend, uncomment below
    ble_set_pins(6, 7);
//    ble_set_name("Blend");

    // Init. and start BLE library.
    ble_begin();
}

void loop()
{
    if (ble_available() )
    {
        uint8_t temp;
        while ( ble_available() )
        {
            temp = ble_read();
            if('o' == temp)
            {
                temp = ble_read();
                if('n' == temp)
                {
                    digitalWrite(13, HIGH);
                    ble_write_bytes((unsigned char *)"LED on", 6);
                }
                else if('f' == temp && 'f' == ble_read())
                {
                    digitalWrite(13, LOW);
                    ble_write_bytes((unsigned char *)"LED off", 7);
                }
            }
        }
    }
//    delay(100);
  digitalWrite(12,LOW);
    ble_do_events();  
}

void BtnDownCB()
{   
    PRR0 = 0x00;  // Power Reduction Register: open timer
    PRR1 = 0x00;
  digitalWrite(12,HIGH);
//    delay(100);  //jitters elimination
//    if(LOW == digitalRead(1))
    {
        if ( ble_connected() )
        {
            ble_write_bytes((unsigned char *)"Button down", 11);
        }
    }
}
