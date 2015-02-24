#ifndef FANCONTROLLER_H
#define FANCONTROLLER_H

#include <cstdint>

class FanController
{

public:
    typedef void (*InterruptFunc)();

protected:
    uint8_t mOnOffPin;
    uint8_t mPwmPin;
    uint8_t mTachPin;
    uint8_t mPower;
    uint8_t mMode;
    uint32_t mTachCount;
    int32_t  mTachTimer;

    // Interrupt functions
    static volatile int counter[12];

    static void interruptD0();
    static void interruptD1();
    static void interruptD2();
    static void interruptD3();
    static void interruptD4();
    static void interruptA0();
    static void interruptA1();
    static void interruptA3();
    static void interruptA4();
    static void interruptA5();
    static void interruptA6();
    static void interruptA7();





    // Helper Functions
    void setOn(bool ison);
    int pinToIndex();
    void setInterrupt();


public:
    FanController( int onOffPin, int pwmPin, int tachPin);
    FanController();


    // Initalization
    void setup( int onOffPin, int pwmPin, int tachPin );
    void setup();


    // Fan speed control
    void setSpeed(int speed);
    int getSpeed();


    // On / off control
    bool isOn();
    void on();
    void off();


    // Fan speed measurement
    void startMeasureTach();
    void stopMeasureTach();
    int getRPM();
    void resetTach();

};

#endif // FANCONTROLLER_H
