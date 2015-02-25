#ifndef __FANLOGIC__
#define __FANLOGIC__

#include <cstdint>
#include "LoopArray.h"
#include "Timer.h"

class TempSensor;

class FanLogic
{
public:

    static const int SampleTime = 2500;



    enum FanLogicModes
    {
        FLM_Standard,       // Normal checking mode (make a check every few seconds)
        FLM_PowerDown,      // Continue checking until temp is ok, then go to low power mode
        FLM_LowPower,       // Check every few minutes. If detecting rise in temp then change mode
        FLM_ForceOn,        // Keep the fans on at the given power value. If however temp is too high ok to increase fan speed
        FLM_ForceOff        // Try to turn the fans off. If temp too high keep the fans on.
    };


// PID Values
    float       mIterm;
    float       mLastInput;
    float       mKi;
    float       mKd;
    float       mKp;
    float       mGoalTemp;

    uint8_t     calculatePID();

    uint8_t    mMode;


    // Used to calculate change in temp data
    // data is stored in raw temp format. Uses less memory and does not require floating point math
    LoopArray<int16_t, 4>   mTemps;
    LoopArray<int16_t, 4>   mTimes;

    LoopArray<int16_t, 4>   mLongTemps;

    // Internal timer used by the loop function
    Timer                   mInternalTimer;
    Timer                   mLongTempsTimer;

    // Fan pointers
    FanController * mPush;
    FanController * mPull;

    // Temp sensor related to the fan
    TempSensor * mTemp;

    uint8_t  calculateRequiredPower();

    /**
        Internal function to change the fans speed
        @param[in] speed, set to 0 to completely swith the fans off, send 1 for lowest power
    */
    void     setFanPower(uint8_t power);


    uint8_t  getFanPower();

    /**
        Adds a temperature into the correct slot in the above array
        @param[in] temp raw value
        @param[id] Hidden value. Autmatically insertes the current milliseconds
    */
    void     addTemp( int16_t temp );

    void init();

    static TempSensor * mAmbient;
public:

    FanLogic();
    FanLogic( FanController & oneFan, TempSensor & sensor );
    FanLogic( FanController & push, FanController & pull, TempSensor & sensor );


    static void initalizeFanLogicControllers( TempSensor & ambient );

    void setup();
    void loop();



    void setTuningParameters( float kp, float ki, float kd );

    // Starts more freq checking of the temp sensor
    void setPowerOnMode();

    // Sets the temp sensors to start checking at a slower rate
    void setPowerDownMode();


    void forceOn(int amount = 0); //0 = Auto

    void forceOff();

};














#endif // __FANLOGIC__
