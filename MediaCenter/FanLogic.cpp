#include <cstdint>
#include "application.h"
#include "FanController.h"
#include "TempSensor.h"
#include "FanLogic.h"
#include "OneWireQue.h"


const int16_t EmptyTemp = 0xC000;

const int16_t MaxTemp = 20 << 4;

TempSensor * FanLogic::mAmbient = nullptr;

FanLogic::FanLogic() :
    mPush(nullptr),
    mPull(nullptr),
    mMode(FanLogic::FLM_Standard),
    mNextEntry(0)
{
    init();
}


FanLogic::FanLogic( FanController & oneFan, TempSensor & sensor ) :
    mPush(&oneFan),
    mPull(nullptr),
    mMode(FanLogic::FLM_Standard),
    mNextEntry(0),
    mTemp(&sensor)
{
    init();
}

FanLogic::FanLogic( FanController & push, FanController & pull, TempSensor & sensor ) :
    mPush(&push),
    mPull(&pull),
    mMode(FanLogic::FLM_Standard),
    mNextEntry(0),
    mTemp(&sensor)
{
    init();
}

void FanLogic::init()
{
    for( int i=0; i < NbrTempPoints; ++i)
    {
        mTemps[i] = EmptyTemp; //Prevent 0 temperature recordings
        mTimes[i] = (i+1) * 2; // Prevent division by 0 at startup
    }



}

void FanLogic::initalizeFanLogicControllers( TempSensor & ambient )
{
    mAmbient = &ambient;
    int16_t t;
    mAmbient->requestTempRaw(t, 1); // Force an update to the ambient temperature
}


void FanLogic::setup()
{
    // Do nothing for now
}

void FanLogic::loop()
{
    // Only run the loop every few seconds

    if( millis() - mInternalTimer < 4000 )
        return;

    mInternalTimer = millis();


    uint32_t  maxStale = 2000;
    uint8_t newFanSpeed;
    newFanSpeed = calculateRequiredPower();


    setFanPower(newFanSpeed);

    switch( mMode )
    {

    case FLM_PowerDown: //Also FLM_ForceOff...
        if( newFanSpeed == 0 )
        {
            mMode = FLM_LowPower;
        }
    case FLM_ForceOn:
    case FLM_Standard:
        maxStale = 6000;    // 6 seconds
        break;

    case FLM_LowPower:
        maxStale = 1000 * 60 * 5; // 5 minutes
        break;
    }

    int16_t rawtemp = 0;
    if( mTemp->requestTempRaw( rawtemp, maxStale ) == true )
    {
        addTemp( rawtemp );
    }

}


void     FanLogic::addTemp( int16_t temp )
{
    mTemps[mNextEntry] = temp;
    mTimes[mNextEntry] = millis();
    mNextEntry++;
    if( mNextEntry >= NbrTempPoints )
        mNextEntry = 0;
}

uint8_t FanLogic::getTempIndice(uint8_t index)
{

    // Oldest value is going to be pointed at by mNextEntry
    int value = mNextEntry + index;
    if( value >= NbrTempPoints )
        return value - NbrTempPoints;
    else
        return value;
}

uint8_t FanLogic::getFanPower()
{
    if( !mPush->isOn() )
        return 0;

    return mPush->getSpeed();
}

void FanLogic::setFanPower(uint8_t power)
{
    if( power == 0)
    {
        if(mPush)
            mPush->off();


        if(mPull)
            mPull->off();
    }
    else
    {
        if(mPush)
        {
            mPush->setSpeed(power);
            if( !mPush->isOn() )
                mPush->on();
        }


        if(mPull)
        {
            mPull->setSpeed(power);
            if( !mPull->isOn() )
                mPull->on();
        }

    }


}

uint8_t  FanLogic::calculateRequiredPower()
{
    const int Last = NbrTempPoints - 1;
    const int First = 0;

    // Does not worry at the accuracy of the temp. That is checked else where.
    // This function only worries about what it currently knows and the trend that
    // it is seeing.

    // Function looks at the slope of the temp. If the temp is increasing (positive slope)
    // we want to increase fan speed to bring the rate of slope increase as small as possible.Or even
    // negative.
    // IF the slope is decreasing then try to keep the slope negative with as little fan
    // use as possible.

    // A few gotchas: if the temp is getting too hot then ignore slope calculations and work
    // on temp to bring the temperature back into line.

    uint8_t first = getTempIndice(First);
    uint8_t last  = getTempIndice(Last);
    int currentSpeed = getFanPower();
    int16_t diff = OneWireQue::rawWholePart(mTemps[last]) - mAmbient->getRawWholePart();

    if( diff > MaxTemp )
    {
        currentSpeed += 70;
        return (currentSpeed > 255 ? 255 : currentSpeed);
    }
    if( diff == 0 )
    {
        // If ambient temp and measured temp are the same turn off the fans
        return 0;
    }


    float slope = OneWireQue::convertRawTempToC(mTemps[last] - mTemps[first]) / (mTimes[last] - mTimes[first]); //Deg C / ms
    int16_t powerAdjustment = diff * 2;

    if( slope < -0.0001 )
    {
        powerAdjustment += -10;
        //Temp is dropping
        first = getTempIndice(Last-1);
        float slope2 = OneWireQue::convertRawTempToC(mTemps[last] - mTemps[first]) / (mTimes[last] - mTimes[first]);


        if( slope2 > slope )
            // The slope of the most recent measurements is actually less negative, don't decrease fan speed too much
        {
            powerAdjustment += 5;
        }
        else if (slope2 < slope )
            // Slope negative measurement is more negative than thought, can decrease fan speed more
        {
            powerAdjustment -= 5;
        }
        else
            // Slope is about the same don't make any fine tuning
        {
        }

        return currentSpeed + powerAdjustment;

    }
    else if( slope >= -0.0001 && slope <= 0.0001 )
    {
        // Temp is not changing
        return currentSpeed;
    }
    if( slope > 0.0001 )
    {
        // Temp is increasing
        powerAdjustment += 10;
        //Temp is dropping
        first = getTempIndice(Last-1);
        float slope2 = OneWireQue::convertRawTempToC(mTemps[last] - mTemps[first]) / (mTimes[last] - mTimes[first]);


        if( slope2 > slope )
            // The slope of the most recent measurements is actually more positive , increase fan speed more
        {
            powerAdjustment += 5;
        }
        else if (slope2 < slope )
            // Slope positive measurement is less positive than thought, can decrease fan speed more
        {
            powerAdjustment -= 5;
        }
        else
            // Slope is about the same don't make any fine tuning
        {
        }

        return currentSpeed + powerAdjustment;
    }

}