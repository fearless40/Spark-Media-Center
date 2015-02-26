

#ifndef __TIMERCLASS__
#define __TIMERCLASS__



class Timer
{
protected:

    long    mTimeStart;

public:

    Timer() : mTimeStart(0)
    {}

    Timer( const Timer & timer )
    {
        mTimeStart = timer.mTimeStart;
    }

    Timer & operator = ( const Timer & timer )
    {
        mTimeStart = timer.mTimeStart;
        return *this;
    }

    void start()
    {
        mTimeStart = millis();
    }

    long elapsed()
    {
        return millis() - mTimeStart;
    }

    bool once( long value )
    {
        if( elapsed() >= value )
            return true;

        return false;
    }

    bool interval( long value )
    {
        if( elapsed() >= value )
        {
            mTimeStart = millis();
            return true;
        }

        return false;
    }

};

#endif
