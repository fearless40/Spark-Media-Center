#include "application.h"
#include "FanController.h"
#include "FanLogic.h"
#include "TempSensor.h"
#include "OneWireQue.h"
#include "OneWire.h"



FanController AmplifierPush(D5, A7, A3);
FanController AmplifierPull(D5, A7, A4);
FanController RecieverFan( D6, A5, A1);
FanController PowerFan(D7, A6, A2);

TempSensor AmbientTemp;
TempSensor AmplifierTemp;
TempSensor RecieverTemp;
TempSensor PowerTemp;

FanLogic    AmplifierLogic( AmplifierPush, AmplifierPull, AmplifierTemp );
FanLogic    RecieverLogic( RecieverFan, RecieverTemp );
FanLogic    PowerLogic( PowerFan, PowerTemp );

char   stats[512];
char   stats2[512];
char   stats3[128];


float tempInC = 0;
int fanSpeed = 0;

int milli = 0;

typedef uint8_t ROM[8];

/*
void ReadSingleRom
{
    OneWire one(D4);
    ROM read;
    one.reset();
    one.write(0x33);
    one.read_bytes(read,8);

    sprintf( stats, "High: %u %u %u %u %u %u %u %u", read[7], read[6], read[5], read[4], read[3], read[2], read[1], read[0]);
}
*/

int ChangeInputs(String data)
{
    float kp, ki, kd;
    data.toCharArray( stats3, 128 );
    sscanf( stats3, "%f %f %f", &kp, &ki, &kd );



    AmplifierLogic.setTuningParameters(kp,ki,kd);

    sprintf( stats3, "kp:%f ki:%f kd:%f", kp, ki, kd);
}

void setup()
{

    Spark.variable("Stats", stats, STRING );
    Spark.variable("Stats2", stats2, STRING );

    OneWireQue::setup(D4, 4);

    //OneWireQue::makeRom(rom, OneWireQue::MRF_RandMin, 25, 25, 0, 2000);

    // Ambient temperature rom code
    ROM rom = {40,255,140,196,99,20,1,93};

    AmbientTemp = TempSensor::NewSensor(rom, "Ambient");
    FanLogic::initalizeFanLogicControllers(AmbientTemp);

    // Amplifier temperature rom code
    ROM rom2 = {40,255,104,251,99,20,1,74};

    AmplifierTemp = TempSensor::NewSensor(rom2, "Amp");
    //OneWireQue::makeRom(rom, OneWireQue::MRF_GoUp | OneWireQue::MRF_GoDown, 60, 25, 5, 2000);
    //temp = TempSensor::NewSensor(rom,"Temp");

    ROM rec = {40,255,181,246,99,20,2,135};
    RecieverTemp = TempSensor::NewSensor(rec, "Rec");


    ROM pow = {40,255,71,196,99,20,1,15};
    PowerTemp = TempSensor::NewSensor(pow, "Power");


    AmplifierPush.setup();
    AmplifierPull.setup();
    PowerFan.setup();
    RecieverFan.setup();

    Spark.variable("Stats3", stats3,STRING);
    Spark.variable("Stats", stats, STRING );
    Spark.variable("Stats2", stats2, STRING );
    Spark.function("Change", ChangeInputs );
    //Spark.variable("RPM", &tachs, INT);
    milli = millis();

}


int fanNbr = 0;
int mode = 0;

FanController & getFanByNumber(int nbr)
{
switch (nbr)
{
case 0:
    return AmplifierPush;
case 1:
    return RecieverFan;
case 2:
    return PowerFan;
}

}

void loop()
{
float t;
    OneWireQue::loop();
    AmbientTemp.requestTempInC(t, 1000);
//    AmplifierTemp.requestTempInC(t,1000);
//    PowerTemp.requestTempInC(t,1000);
//    RecieverTemp.requestTempInC(t,1000);
    sprintf(stats, "Ambient: %f  Amp: %f  Rec: %f  Power: %f", AmbientTemp.getTempInC(), AmplifierTemp.getTempInC(), RecieverTemp.getTempInC(), PowerTemp.getTempInC());

/*
    if( millis() - milli > 1000 )
    {
        FanController & fc = getFanByNumber(fanNbr);
        if( mode == 0 )
        {
            fc.setSpeed(30);
            fc.on();
            //fc.setSpeed(30);
            mode = 1;
        }
        else if( mode == 1 )
        {
            fc.off();
            mode = 0;
            ++fanNbr;
            if( fanNbr >= 3)
                fanNbr = 0;
        }

        milli = millis();

    }
*/



    AmplifierLogic.loop();
    RecieverLogic.loop();
    PowerLogic.loop();
    AmplifierPush.off();
    RecieverFan.off();
    PowerFan.off();

    sprintf( stats2, "Amp Speed: %u  Rec Speed: %u  Power Speed: %u",   AmplifierPush.isOn() ? AmplifierPush.getSpeed() : 0,
                                                                        RecieverFan.isOn() ? RecieverFan.getSpeed() : 0,
                                                                        PowerFan.isOn() ? PowerFan.getSpeed() : 0);

}


/*    if( millis() - milli > 1000 )
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

*/


/*
float git(int index)
{
    return OneWireQue::convertRawTempToC(fl.mTemps[fl.getTempIndice(index)]);
}
*/


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

//sprintf(stats2, "Ambient: %f Measured: %f FanSpeed: %u", t, temp.getTempInC(), (fan1.isOn() == true ? fan1.getSpeed() : 0));
    //sprintf(stats2, "Newest: %f, Temp 1: %f, Temp 2: %f, Oldest:%f   Ambient: %f Measured: %f FanSpeed: %u, Fan RPM: %u", git(3), git(2), git(1), git(0), t, temp.getTempInC(), (fan1.isOn() == true ? fan1.getSpeed() : 0), fan1.getRPM() );


    /*int16_t t;
    OneWireQue::loop();
    ambient.requestTempRaw(t,2000);
    fl.loop();
    tempInC = temp.getTempInC();
    fanSpeed = fan1.getSpeed();

    sprintf( stats, "Ambient: %f  Temp: %f  FanSpeed: %u", ambient.getTempInC(), temp.getTempInC(), fan1.getSpeed() );
    */
