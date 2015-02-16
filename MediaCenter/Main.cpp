#include "application.h"
#include "FanController.h"




FanController fan1;
int timer = 0;
int timer2 = 0;
int tachs;


void setup() {
    timer = millis();
    fan1.setup( D7, A6, A0 );
    fan1.setSpeed(0);
    fan1.on();
    fan1.startMeasureTach();
    Spark.variable("RPM", &tachs, INT);
}

int speed2 = 0;

void loop() {

if( millis() - timer2 >= 1000 ){
  fan1.stopMeasureTach();
  tachs = fan1.getRPM();
  //tDigitalRead = (tachCounter * 60) / 2;

  timer2 = millis();

  fan1.startMeasureTach();
}

if( millis() - timer > 3000 )
{
  int speed = fan1.getSpeed();
  speed2 += 10;
  if( speed2 > 255 ) speed2 = 0;
  fan1.setSpeed( speed2 );
  timer = millis();
  //analogWrite(D0, speed2);
  //analogWrite(A6, speed2);
  //analogWrite(A7, speed2);
}

}
