#include "application.h"
#include "OneWire.h"
#include "OneWireQue.h"
#include <cstring>

namespace OneWireQue
{


/// The max rate probes can be queried
const int   MaxProbeUpdate = 1000; // milli seconds since last request to read

/// Every 10 minutes make sure a probe is accessed
const int   LongestNoProbeAccess = 10 * 60 * 1000; // 10 minutes


enum class ProbeStatus : uint8_t
{
    Idle,
    Running,
    Request
};

enum RomCommands
{
    RC_Match   = 0x55,
    RC_Read    = 0x33,
    RC_Search  = 0xF0,
    RC_Skip    = 0xCC,
    RC_ASEARCH = 0xEC,
};

enum TempCommands
{
    TC_Convert = 0x44,
    TC_Write   = 0x4E,
    TC_Read    = 0xBE,
    TC_Copy    = 0x48,
    TC_Recall  = 0xB8,
};


struct TempProbes
{
    uint32_t    lastUpdate;
    uint16_t    rawValue;
    uint8_t     rom[8];
    ProbeStatus status;
    char        name[8];
};

/// The number of probes
uint8_t         mNbrProbes;

/// Array of probes
TempProbes  *   mProbes;


uint16_t        mProbeCurrent;

OneWire * mWire;

ifdef SIMULATION
void makeTemp( uint8_t * data, uint8_t id )
{
    // Uses rom data to figure out how to generate the temp
    // No check to see if the id is bad.
    TempProbes p = mProbes[id];
    switch (p.rom[0])   //Switch on genFunc
    {
        case 0:

    }



}


void makeRom( uint8_t * rom, uint8_t genFunc, uint8_t maxTemp, uint8_t minTemp, uint8_t rateOfChange )
{
    rom[0] = genFunc;
    rom[1] = maxTemp;
    rom[2] = minTemp;
    rom[3] = rateOfChange;
    // rom[4-7] = used internally by makeTemp
}


bool setup( int pin, int nbrProbes )
{
    mNbrProbes = nbrProbes;
    if(  mNbrProbes < 0 || mNbrProbes > 50 )
        return false;

#ifndef SIMULATION
    mWire = new OneWire( pin );


    if( !mWire )
        return false;

#endif

    mProbes = new TempProbes[mNbrProbes];

    if( !mProbes )
        return false;

    memset( mProbes, 0 , sizeof( TempProbes) * mNbrProbes );

    mProbeCurrent = 0;

    return true;
}

int registerProbe( uint8_t * rom, const char * name )
{
    if( mProbeCurrent >= mNbrProbes )
        mProbeCurrent = 0;

    memcpy( mProbes[mProbeCurrent].rom, rom, ROM_ID_SIZE );
    strcpy( mProbes[mProbeCurrent].name, name );

    mProbes[mProbeCurrent].status = ProbeStatus::Idle;

    int ret = mProbeCurrent;
    mProbeCurrent++;

    return ret;
}

void selectRom( TempProbes & probe )
{
#ifndef SIMULATION

    // Reset the one wire device
    mWire->reset();

    // Tell the device who the next command is for
    mWire->write_bit(RC_Match);

    // Send the rom that we are looking for
    mWire->write_bytes(probe.rom, 8, 0);

#endif
}

void loop()
{
    // Raw data that is read from the bus
    uint8_t rawData[9];

    for( uint8_t looper = 0; looper < mNbrProbes; ++looper )
    {
        TempProbes & probe = mProbes[looper];

        if( probe.status == ProbeStatus::Idle && ( (millis() - probe.lastUpdate) > LongestNoProbeAccess))
        {
            probe.status = ProbeStatus::Request;
        }

        if( probe.status == ProbeStatus::Running && (millis() - probe.lastUpdate ) > MaxProbeUpdate )
        {
            //Probe is now ready for reading
            probe.status = ProbeStatus::Idle;
            selectRom(probe);
            mWire->write_bit(TC_Read);
            mWire->read_bytes(rawData, 9);

            uint8_t high = rawData[1];
            uint8_t low = rawData[2];
            probe.rawValue = (high << 8) | low;
            probe.lastUpdate = millis();
        }

        if( probe.status == ProbeStatus::Request )
        {
            selectRom(probe);
            mWire->write_bit(TC_Convert);
            probe.status = ProbeStatus::Running;
            probe.lastUpdate = millis();
        }

    }
}

bool isIDOk( int id )
{
    return (id >=0 && id < mNbrProbes );
}

bool isTempReady( int id, int maxStale )
{
    // Prevent reading from random memory
    if( !isIDOk(id) )
        return false;

    TempProbes & probe = mProbes[id];

    if( probe.status == ProbeStatus::Running )
    {
        return false;
    }
    else if( probe.status == ProbeStatus::Idle )
    {
        if( maxStale > (millis() - probe.lastUpdate) )
            return false;
        else
            return true;
    }
    else if( probe.status == ProbeStatus::Request )
        return false;

    return true;
}

bool requestTempRaw( int id, int16_t & temp,  uint32_t maxStale )
{
    temp = 0;

    if( !isIDOk(id) )
        return false;   //Not a valid id

    // Always return a value of some sort
    temp = mProbes[id].rawValue;

    if( isTempReady( id, maxStale ) )
    {
        // Return that the value is uptodate
        return true;
    }
    else if ( mProbes[id].status == ProbeStatus::Request || mProbes[id].status == ProbeStatus::Running )
    {
        // Value is not uptodate.
        // But do not request a new value as we are working on it already.
        return false;
    }
    else
    {
        mProbes[id].status = ProbeStatus::Request;

        // Return the value is not uptodate
        return false;
    }


}

bool requestTempInC( int id, float & temp,  uint32_t maxStale )
{
    int16_t raw;
    bool ret = requestTempRaw( id, raw, maxStale );
    temp = convertRawTempToC(raw);
    return ret;
}

bool requestTempInF( int id, float & temp,  uint32_t maxStale  )
{
    int16_t raw;
    bool ret = requestTempRaw( id, raw, maxStale );
    temp = convertRawTempToF(raw);
    return ret;
}

int16_t getTempRaw( int id )
{
    if( !isIDOk(id) )
        return 0;

    return mProbes[id].rawValue;
}

float getTempInC( int id )
{
    if( !isIDOk(id) )
        return 0.0f;

    return (float) (mProbes[id].rawValue / 16);
}

float getTempInF( int id )
{
    if( !isIDOk(id) )
        return 0.0f;

    return (( (float) (mProbes[id].rawValue / 16) * 9.0f) / 5.0f ) + 32.0f;
}

uint32_t    getElaspedTimeSinceUpdate(int id)
{
    if( !isIDOk(id) )
        return 0;

    return mProbes[id].lastUpdate;
}

float convertRawTempToC( int16_t rawtemp )
{
    return (float) (rawtemp / 16);
}

float convertRawTempToF( int16_t rawtemp )
{
    return (((rawtemp/16) * 9.0f) / 5.0f) + 32.0f;
}

int16_t rawWholePart( int16_t rawTemp )
{
    /// Adapted from: http://playground.arduino.cc/Learning/OneWire
    int16_t SignBit, Tc_100;
    SignBit = rawTemp & 0x8000;  // test most sig bit

    if (SignBit) // negative
    {
        rawTemp = (rawTemp ^ 0xffff) + 1; // 2's comp
    }

    Tc_100 = (6 * rawTemp) + rawTemp / 4;    // multiply by (100 * 0.0625) or 6.25

    return Tc_100 / 100;  // separate off the whole and fractional portions
}

int16_t rawFracPart( int16_t rawTemp )
{
    /// Adapted from: http://playground.arduino.cc/Learning/OneWire
    int16_t SignBit, Tc_100;
    SignBit = rawTemp & 0x8000;  // test most sig bit

    if (SignBit) // negative
    {
        rawTemp = (rawTemp ^ 0xffff) + 1; // 2's comp
    }

    Tc_100 = (6 * rawTemp) + rawTemp / 4;    // multiply by (100 * 0.0625) or 6.25


    return Tc_100 % 100;
}

} //End Namespaces


