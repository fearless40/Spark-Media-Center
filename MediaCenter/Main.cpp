#include "application.h"
#include "FanController.h"
#include "FanLogic.h"
#include "TempSensor.h"
#include "OneWireQue.h"
#include "OneWire.h"



FanController fan1;
TempSensor ambient;
TempSensor temp;
char   stats[512];
char   stats2[512];

FanLogic fl(fan1, temp);
float tempInC = 0;
int fanSpeed = 0;
OneWire one(D4);
int milli = 0;
//uint8_t rom[8];

typedef uint8_t ROM[8];

void setup()
{
    ROM read;
    one.reset();
    one.write(0x33);
    one.read_bytes(read,8);

    sprintf( stats, "High: %u %u %u %u %u %u %u %u", read[7], read[6], read[5], read[4], read[3], read[2], read[1], read[0]);

        Spark.variable("Stats", stats, STRING );
    Spark.variable("Stats2", stats2, STRING );

return;
    OneWireQue::setup(D4, 2);

    //OneWireQue::makeRom(rom, OneWireQue::MRF_RandMin, 25, 25, 0, 2000);

    ROM rom = {40,255,104,251,99,20,1,74};

    ambient = TempSensor::NewSensor(rom, "Ambient");
    FanLogic::initalizeFanLogicControllers(ambient);

    ROM rom2 = {40,255,181,246,99,20,2,135};

    temp = TempSensor::NewSensor(rom2, "Temp");
    //OneWireQue::makeRom(rom, OneWireQue::MRF_GoUp | OneWireQue::MRF_GoDown, 60, 25, 5, 2000);
    //temp = TempSensor::NewSensor(rom,"Temp");

    fan1.setup( D5, A7, A0 );
    //fan1.startMeasureTach();

    Spark.variable("FanSpeed", &fanSpeed, INT);
    Spark.variable("Temp", &tempInC, DOUBLE);
    Spark.variable("Stats", stats, STRING );
    Spark.variable("Stats2", stats2, STRING );
    //Spark.variable("RPM", &tachs, INT);
    milli = millis();

}

int speed2 = 0;
int mode = 0;

float git(int index)
{
    return OneWireQue::convertRawTempToC(fl.mTemps[fl.getTempIndice(index)]);
}

void loop()
{
return;
    uint8_t data[9];
    float t;

    OneWireQue::loop();
    fl.loop();

    ambient.requestTempInC(t, 3000);

    //sprintf(stats2, "Ambient: %f Measured: %f FanSpeed: %u", t, temp.getTempInC(), (fan1.isOn() == true ? fan1.getSpeed() : 0));
    sprintf(stats2, "Newest: %f, Temp 1: %f, Temp 2: %f, Oldest:%f   Ambient: %f Measured: %f FanSpeed: %u, Fan RPM: %u", git(3), git(2), git(1), git(0), t, temp.getTempInC(), (fan1.isOn() == true ? fan1.getSpeed() : 0), fan1.getRPM() );



    if( millis() - milli > 1000 )
    {
        if( mode == 0 )
        {
            fan1.startMeasureTach();
            mode = 1;
        }
        else if( mode == 1 )
        {
            fan1.stopMeasureTach();
            mode = 0;
        }

        milli = millis();

    }


    /*int16_t t;
    OneWireQue::loop();
    ambient.requestTempRaw(t,2000);
    fl.loop();
    tempInC = temp.getTempInC();
    fanSpeed = fan1.getSpeed();

    sprintf( stats, "Ambient: %f  Temp: %f  FanSpeed: %u", ambient.getTempInC(), temp.getTempInC(), fan1.getSpeed() );
    */


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
