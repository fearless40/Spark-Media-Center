#ifndef __ONEWIREQUE__
#define __ONEWIREQUE__

// Hardwired as I know exactly what is connected to this particular spark core
#undef SIMULATION

/**
    A simple que of one wire devices that works in a simple fashion.
    Why is it written as a 'C' API?
        The reason is one for simplicity.

    Yes I know this could easily be done in a standard style C++ class. However in this case you
        really are only going to have one of these in a program. I find it highly unlikely that
        you are using multiple pins on your precious device to run multiple one wire deivices!
        OneWire devices are designed to use one digital PIN. If you need to create more than
        one of this class than feel free to change it into a standard C++ class!
*/

namespace OneWireQue
{
const int ROM_ENTRIES = 8;
const int ROM_ID_SIZE = sizeof(uint8_t) * ROM_ENTRIES;


#ifdef SIMULATION

enum MakeRomFlags
{
    MRF_GoUp        = 0x01,
    MRF_GoDown      = 0x02,
    MRF_RandMin     = 0x04,
    MRF_RandMax     = 0x08,
    MRF_MinToMaxNoMiddle = 0x10
};


/**
    Used Internally to make a fake temperature
    @param[out] the generated data
    @param[in] id of the temp probe
*/
void makeTemp( uint8_t * data, uint8_t id );

/**
    Make a rom that describes how the temp should fluctuate with time
    @param[out] a field of 8 uint8_t bytes. The generated rom
    @param[in] genFunc (see below for more details)
    @param[in] max temperature to generate
    @param[in] min temperature to generate
    @param[in] rate of change (see genFunc below)

    @details
        genFunc Flags
        Bit 0: Go up
        Bit 1: Go down
        Bit 2: Minimal Random variation
        Bit 3: Max Random variation
        Bit 4: Min Max, No in between variation
*/

void makeRom( uint8_t * rom, uint8_t genFunc, uint8_t maxTemp, uint8_t minTemp, int8_t rateOfChange, uint16_t freq );
#endif

/**
    Call once to setup the one wire interface
    @param[in] The pin that the OneWire interface is hooked up to
    @param[in] The number of probes on the Interface
    @return true if ok. False if not.
*/

bool setup( int pin, int nbrProbes );

/**
    Call each loop of your main loop for processing
*/

void loop();

/**
    Add a new probe to the list
    @param[in] One wire Rom ID
    @param[in] Internal name
*/
int registerProbe( uint8_t * rom, const char * name );

/**
    Is a probe ready for new request
    @param[in] id of the prove
    @param[in] max acceptable staleness of the data in milliseconds
    @return True if ready for a new request
*/
bool isTempReady( int id, int maxStale = 0 );

/**
    Request a temperature from the device. Always returns the temperature
    @param[in] if of the probe
    @param[out] temp in raw value
    @param[in] amount of time for the value to be valid before requesting a new value
    @return true if the value is not stale. false if otherwise
*/
bool requestTempRaw( int id, int16_t & temp,  uint32_t maxStale = 0 );

/**
    Request a temperature from the device. Always returns the temperature
    @param[in] if of the probe
    @param[out] temp in celcius
    @param[in] amount of time for the value to be valid before requesting a new value
    @return true if the value is not stale. false if otherwise
*/
bool requestTempInC( int id, float & temp,  uint32_t maxStale = 0 );

/**
    Request a temperature from the device. Always returns the temperature
    @param[in] if of the probe
    @param[out] temp in farienheit
    @param[in] amount of time for the value to be valid before requesting a new value
    @return true if the value is not stale. false if otherwise
*/
bool requestTempInF( int id, float & temp,  uint32_t maxStale = 0 );


/**
    Only returns the current temp value. WILL not force update
    @param[in] id of probe
    @return temp in C, does not check for data staleness
*/

float getTempInC( int id );


/**
    Only returns the current temp value. WILL not force update
    @param[in] id of probe
    @return temp in F, does not check for data staleness
*/
float getTempInF( int id );


int16_t getTempRaw( int id );

/**
    Elapsed time in milli seconds since the probe was checked
    @return time in milli seconds
*/
uint32_t    getElaspedTimeSinceUpdate(int id);

/**
    Simple conversion helper function for working with raw values
    @param[in] raw temp value
    @return temp in C
*/
float convertRawTempToC( int16_t rawtemp );

/**
    Simple conversion helper function for working with raw values
    @param[in] raw temp value
    @return temp in F
*/
float convertRawTempToF( int16_t rawtemp );

int16_t rawWholePart( int16_t rawTemp );

int16_t rawFracPart( int16_t rawTemp );
}





#endif
