
#include "BDRB_LedLib.h"

BDRB_LedLib myLed(LED_BUILTIN);


void setup() {
  // put your setup code here, to run once:
  if (!Serial)
    {
      Serial.begin(9600);
      delay(5000);      
    }

//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }
  Serial.println("SERIAL STARTED");
  
  myLed.RunLedLib(); //Nodig om statemachine te initialiseren
}

void loop() {
  // always run this command:
  myLed.RunLedLib();

  //Serial.println("RunledLib ok");
  if(millis()>0UL && millis()<5000UL)
    {
      myLed.Knipper(100,100);
    }
  if(millis()>6000UL && millis()<11000UL)
    {
      myLed.Knipper(250);
    } 
  if(millis()>12000UL && millis()<17000UL)
    {
      myLed.Knipper(50,100, 50, 1000);
    }
  if(millis()>18000UL && millis()<23000UL)
    {
      myLed.FadeIn(1000, 250);
    }
  if(millis()>24000UL && millis()<29000UL)
    {
      myLed.FadeOut(1000, 500);
    }
  if(millis()>30000UL && millis()<35000UL)
    {
      myLed.FadeInOut(750, 250);
    }
  if(millis()>36000UL && millis()<41000UL)
    {
      myLed.FadeInOut(500, 1000, 250);
    }    

   if(millis()>43000ULL)
    {
      myLed.Knipper(50,100, 50, 1000);
    }    
}
