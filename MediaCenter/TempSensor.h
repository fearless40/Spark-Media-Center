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

public:

    TempSensor( const TempSensor & t ) : mId(t.mId) { }
    TempSensor( int id ) : mId(id) { }


    TempSensor() : mId(0) {}

    bool    isTempReady()
    {
        return OneWireQue::isTempReady( mId );
    }

    bool    requestTempInC(float & temp, uint32_t maxStale )
    {
        return OneWireQue::requestTempInC(mId, temp, maxStale);
    }


    bool    requestTempInF(float & temp, uint32_t maxStale )
    {

        return OneWireQue::requestTempInF(mId, temp, maxStale);
    }

    bool    requestTempRaw(int16_t & temp, uint32_t maxStale )
    {

        return OneWireQue::requestTempRaw(mId, temp, maxStale);
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
