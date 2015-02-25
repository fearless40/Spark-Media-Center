#include <cstdint>
#include "application.h"
#include "FanController.h"
#include "TempSensor.h"
#include "FanLogic.h"
#include "OneWireQue.h"


const int16_t EmptyTemp = 0xC000;

const int16_t MaxTemp = 50 * 16;  // About 50 degrees centigrade

const int MinTimeToAddNewDataPoint = 2500; // milliseconds

const float AmountAboveAmbient = 2;

const int16_t TempRejectionValue = 20 * 16;

const float OutMax = 255;

const float OutMin = 0;

const int MostRecentTimeSample = FanLogic::NbrTempPoints - 1;


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


    mKi = 0.8;
    mKd = 0.4;
    mKp = 6;

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

void FanLogic::setTuningParameters( float kp, float ki, float kd )
{
    float SampleInSec = SampleTime / 1000;
    mKp = kp / SampleInSec;
    mKd = kd / SampleInSec;
    mKi = ki * SampleInSec;
}

void FanLogic::loop()
{
    // Only run the loop every few seconds
    uint32_t  maxStale = 2000;
    int16_t rawtemp = 0;
    uint8_t newFanSpeed;

    if( mTemp->requestTempRaw( rawtemp, maxStale ) == true )
    {

        if( millis() - mInternalTimer < SampleTime )
            return;

        // Run logic every few seconds
        mInternalTimer = millis();

        addTemp( rawtemp );


        newFanSpeed = calculateRequiredPower();


        setFanPower(newFanSpeed);
    }
    else
    {
        return; // No new data so not worth checking fan speed
    }


    /*    if( millis() - mInternalTimer < 4000 )
            return;

        mInternalTimer = millis();
    */



    /*    switch( mMode )
        {

        case FLM_PowerDown: //Also FLM_ForceOff...
            if( newFanSpeed == 0 )
            {
                mMode = FLM_LowPower;
            }
        case FLM_ForceOn:
        case FLM_Standard:
            maxStale = 2000;    // 2 seconds
            break;

        case FLM_LowPower:
            maxStale = 1000 * 60 * 5; // 5 minutes
            break;
        }
    */

}


void     FanLogic::addTemp( int16_t temp )
{

    // Also want to throw out garbage data if we receive garbage data store the
    // last data point.
    // Garbage data is determied as follows:
    //  if abs(temp - mTemps[mNextEntry-1]) > 20 then check it it against

    int16_t priorTemp = mTemps[getTempIndice(NbrTempPoints - 1)];

    if( priorTemp != EmptyTemp && abs(temp - priorTemp) > TempRejectionValue)
    {
        temp = priorTemp;
    }

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

uint8_t     FanLogic::calculatePID()
{

    float input = OneWireQue::convertRawTempToC(mTemps[getTempIndice(MostRecentTimeSample)]);
    float error = input - (mAmbient->getTempInC() + AmountAboveAmbient);  // Above ambient controls acceptable temperatures

    if( error <= 0)
    // If we are cooler than ambient or close to ambient then shut off
    {
        mIterm = 0;
        return 0;
    }

    mIterm += mKi * error;

    if( mIterm > OutMax )
        mIterm = OutMax;

    else if( mIterm < OutMin )
        mIterm = OutMin;

    float dInput = (input - mLastInput);

    float output = mKp * error + mIterm - (mKd * dInput);

    if( output > OutMax )
        output = OutMax;

    else if( output < OutMin )
        output = OutMin;

    mLastInput = input;

    return (uint8_t) output;


}

uint8_t  FanLogic::calculateRequiredPower()
{

    return calculatePID();

    const int Last = NbrTempPoints - 1; // Newest
    const int First = 0;                // Oldest

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
    int16_t currentSpeed = getFanPower();
    int16_t diff = OneWireQue::rawWholePart(mTemps[last]) - mAmbient->getRawWholePart();

    if( diff > MaxTemp )
    {
        currentSpeed += 70;
        return (currentSpeed > 255 ? 255 : currentSpeed);
    }
    if( diff <= 1 )
    {
        // If ambient temp and measured temp are the same turn off the fans
        return 0;
    }


    float slope = OneWireQue::convertRawTempToC(mTemps[last] - mTemps[first]) / (mTimes[last] - mTimes[first]); //Deg C / ms
    int16_t powerAdjustment = 0;

    if( slope < -0.0001 )
    {
        powerAdjustment -= 10;
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

    }
    else if( slope >= -0.0001 && slope <= 0.0001 )
    {
        // Temp is not changing
        powerAdjustment = 0;
    }
    else if( slope > 0.0001 )
    {
        // Temp is increasing
        powerAdjustment += 10;

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
    }

    // PWM duty cycle does not effect fans below 30 percent

    if( currentSpeed < 30 )
    {
        currentSpeed = 30;
    }
    return currentSpeed + powerAdjustment;

}
