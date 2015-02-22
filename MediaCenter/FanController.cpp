#include "FanController.h"
#include <application.h>

volatile int FanController::counter[12];

void FanController::interruptD0()
{
    counter[0]++;
}
void FanController::interruptD1()
{
    counter[1]++;
}
void FanController::interruptD2()
{
    counter[2]++;
}
void FanController::interruptD3()
{
    counter[3]++;
}
void FanController::interruptD4()
{
    counter[4]++;
}
void FanController::interruptA0()
{
    counter[5]++;
}
void FanController::interruptA1()
{
    counter[6]++;
}
void FanController::interruptA3()
{
    counter[7]++;
}
void FanController::interruptA4()
{
    counter[8]++;
}
void FanController::interruptA5()
{
    counter[9]++;
}
void FanController::interruptA6()
{
    counter[10]++;
}
void FanController::interruptA7()
{
    counter[11]++;
}

void FanController::setOn(bool ison)

{
    int low = StatePower & mState;
    int high = ison == true ? 1 : 0;
    mState = (high << 8) + low;
}

void FanController::setPower(int power)
{
    int high = StateOnOff & mState;
    int low = power & StatePower;
    mState = high + low;
}

int FanController::pinToIndex()
{
    switch(mTachPin)
    {
    case D0:
        return 0;
    case D1:
        return 1;
    case D2:
        return 2;
    case D3:
        return 3;
    case D4:
        return 4;
    case A0:
        return 5;
    case A1:
        return 6;
    case A3:
        return 7;
    case A4:
        return 8;
    case A5:
        return 9;
    case A6:
        return 10;
    case A7:
        return 11;
    };
}


/*
FanController::InterruptFunc pinToInterrupt()
{
switch(mTachPin)
{
  case D0:
    return interruptD0;
  case D1:
    return interruptD1;
  case D2:
    return interruptD2;
  case D3:
    return interruptD3;
  case D4:
    return interruptD4;
  case A0:
    return interruptA0;
  case A1:
    return interruptA1;
  case A3:
    return interruptA3;
  case A4:
    return interruptA4;
  case A5:
    return interruptA5;
  case A6:
    return interruptA6;
  case A7:
    return interruptA7;
};
}
*/
void FanController::setInterrupt()
{
    switch(mTachPin)
    {
    case D0:
        attachInterrupt( mTachPin, FanController::interruptD0, RISING );
        return;
    case D1:
        attachInterrupt( mTachPin, FanController::interruptD1, RISING );
        return;
    case D2:
        attachInterrupt( mTachPin, FanController::interruptD2, RISING );
        return;
    case D3:
        attachInterrupt( mTachPin, FanController::interruptD3, RISING );
        return;
    case D4:
        attachInterrupt( mTachPin, FanController::interruptD4, RISING );
        return;
    case A0:
        attachInterrupt( mTachPin, FanController::interruptA0, RISING );
        return;
    case A1:
        attachInterrupt( mTachPin, FanController::interruptA1, RISING );
        return;
    case A3:
        attachInterrupt( mTachPin, FanController::interruptA3, RISING );
        return;
    case A4:
        attachInterrupt( mTachPin, FanController::interruptA3, RISING );
        return;
    case A5:
        attachInterrupt( mTachPin, FanController::interruptA5, RISING );
        return;
    case A6:
        attachInterrupt( mTachPin, FanController::interruptA6, RISING );
        return;
    case A7:
        attachInterrupt( mTachPin, FanController::interruptA7, RISING );
        return;
    };
}


FanController::FanController( int onOffPin, int pwmPin, int tachPin) :
    mOnOffPin( onOffPin ),
    mPwmPin( mPwmPin ),
    mTachPin( tachPin ),
    mState(0)
{}

FanController::FanController() :
    mOnOffPin( 0 ),
    mPwmPin( 0 ),
    mTachPin( 0 ),
    mState(0)
{}

void FanController::setup( int onOffPin, int pwmPin, int tachPin )
{
    mOnOffPin = onOffPin;
    mPwmPin = pwmPin;
    mTachPin = tachPin;
    setup();
}

void FanController::setup()
{
    pinMode( mOnOffPin, OUTPUT );
    pinMode( mPwmPin, OUTPUT );
    pinMode( mTachPin, INPUT_PULLUP );

    setSpeed( 0 );
    off();
}

void FanController::setSpeed(int speed)
{
    if( speed > 255 )
        speed = 255;

    if( speed < 0 )
        speed = 0;

    setPower( speed );
    analogWrite( mPwmPin, speed );
}

bool FanController::isOn()
{
    if( ((mState & StateOnOff)>> 8) == 1 )
        return true;
    else
        return false;
}

int FanController::getSpeed()
{
    return mState & StatePower;
}

void FanController::on()
{
    setOn( true );
    digitalWrite( mOnOffPin, HIGH );
    setSpeed( getSpeed() );
}

void FanController::off()
{
    setOn( false );
    digitalWrite( mOnOffPin, LOW );
}

void FanController::startMeasureTach()
{
    mTachCount = 0;
    counter[pinToIndex()] = 0;
    mTachTimer = millis();
    setInterrupt();

}

void FanController::stopMeasureTach()
{
    detachInterrupt( mTachPin );
    mTachTimer = millis() - mTachTimer;
    mTachCount = counter[pinToIndex()];
}

int FanController::getRPM()
{
    return  ((mTachCount/2)*6000) /mTachTimer;
}

void FanController::resetTach()
{
    mTachCount = 0;
}






