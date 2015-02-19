#ifndef __TEMPSENSOR__
#define __TEMPSENSOR__

#include "OneWireQue.h"

/**
    Simple class to wrap the 'C' style API of OneWireQue in a 'C++' way
*/

class TempSensor
{
protected:

    uint8_t         mId;
    uint32_t        mMaxStale;

public:

    TempSensor( const TempSensor & t ) : mId(t.mId), mMaxStale( t.mMaxStale ) { }
    TempSensor( int id ) : mId(id), mMaxStale(1000 * 30) { }


    TempSensor() : mId(0), mMaxStale(1000 * 30) {}

    bool    isTempReady()
    {
        return OneWireQue::isTempReady( mId );
    }

    bool    requestTempInC(float & temp, uint32_t maxStale )
    {
        mMaxStale = maxStale;
        return OneWireQue::requestTempInC(mId, temp, maxStale);
    }

    bool    requestTempInC(float & temp )
    {
        return OneWireQue::requestTempInC(mId, temp, mMaxStale);
    }

    bool    requestTempInF(float & temp, uint32_t maxStale )
    {
        mMaxStale = maxStale;
        return OneWireQue::requestTempInF(mId, temp, maxStale);
    }

    bool    requestTempInF(float & temp )
    {
        return OneWireQue::requestTempInF(mId, temp, mMaxStale);
    }

    bool    requestTempRaw(int16_t & temp, uint32_t maxStale )
    {
        mMaxStale = maxStale;
        return OneWireQue::requestTempRaw(mId, temp, maxStale);
    }

    bool    requestTempInF(int16_t & temp )
    {
        return OneWireQue::requestTempRaw(mId, temp, mMaxStale);
    }


    float getTempInC( )
    {
        return OneWireQue::getTempInC( mId );
    }

    float getTempInF( int id )
    {
        return OneWireQue::getTempInF( mId );
    }

    int getElasped()
    {
        return OneWireQue::getElaspedTimeSinceUpdate(mId);
    }

    int16_t getRawTemp()
    {
        return OneWireQue::getTempRaw(mId);
    }

    int16_t getRawWholePart()
    {
        return OneWireQue::rawWholePart( OneWireQue::getTempRaw(mId));
    }

    int16_t getRawFracPart()
    {
        return OneWireQue::rawFracPart( OneWireQue::getTempRaw(mId));
    }

    static TempSensor NewSensor( uint8_t * rom, const char * name )
    {
        return TempSensor(OneWireQue::registerProbe(rom, name));
    }


};






#endif // __TEMPSENSOR__
