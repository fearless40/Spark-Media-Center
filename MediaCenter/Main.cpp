#include "application.h"
#include "FanController.h"
#include "FanLogic.h"
#include "TempSensor.h"
#include "OneWireQue.h"




FanController fan1;
TempSensor ambient;
TempSensor temp;
char   stats[512];

FanLogic fl(fan1, temp);
float tempInC = 0;
int fanSpeed = 0;

void setup()
{
    uint8_t rom[8];

    OneWireQue::setup(3, 2);

    OneWireQue::makeRom(rom, OneWireQue::MRF_RandMin, 25, 25, 0, 2000);

    ambient = TempSensor::NewSensor(rom, "Ambient");
    FanLogic::initalizeFanLogicControllers(ambient);

    OneWireQue::makeRom(rom, OneWireQue::MRF_GoUp | OneWireQue::MRF_GoDown, 60, 25, 5, 2000);
    temp = TempSensor::NewSensor(rom,"Temp");

    fan1.setup( D7, A6, A0 );
    //fan1.startMeasureTach();

    Spark.variable("FanSpeed", &fanSpeed, INT);
    Spark.variable("Temp", &tempInC, DOUBLE);
    Spark.variable("Stats", stats, STRING );
    //Spark.variable("RPM", &tachs, INT);
}

int speed2 = 0;

void loop()
{

    int16_t t;
    OneWireQue::loop();
    ambient.requestTempRaw(t,2000);
    fl.loop();
    tempInC = temp.getTempInC();
    fanSpeed = fan1.getSpeed();

    sprintf( stats, "Ambient: %f  Temp: %f  FanSpeed: %u", ambient.getTempInC(), temp.getTempInC(), fan1.getSpeed() );

}


/*if( millis() - timer2 >= 1000 ){
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
}*/
