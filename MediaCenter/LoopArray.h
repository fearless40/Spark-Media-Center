
#ifndef __LOOPARRAYCLASS__
#define __LOOPARRAYCLASS__

/**
    Simple Circular Buffer
    @description Please note that DataType must be a simple type that allows copying. If you use more
                 in depth data structures you will get poor results and poor speed. In that case
                 I recommend that you use a different class

    @param basic type such as (int, uint, int16_t)...
    @param The total size of the array. The array is fixed in size.
*/

template< typename DataType, int NumberEntries >
class LoopArray
{
protected:
    DataType mData[NumberEntries];


public:

    static const int NewestEntryIndex = NumberEntries - 1;
    static const int OldestEntryIndex = 0;

    LoopArray( const DataType & initalvalues )
    {
        fill( initalvalues );
    }

    LoopArray()
    {

    }

    /**
        Fills the array with specified value
        @param The value to fill the array with.
    */
    void fill( const DataType & value )
    {
        for( int loop = 0; loop < NumberEntries; ++loop )
            mData[loop] = value;
    }

    /**
        Adds a value to the end of the array. It throws out the oldest value in the array
        @param The value to add to the end.
    */
    void add( const DataType & value )
    {
        // Will Shift the data over rather than keep a loop variable.
        // The compiler will optimize this as it sees fit to.
        for( int loop = 1; loop < NumberEntries; ++loop )
            mData[loop-1] = mData[loop];

        mData[NumberEntries-1] = value;
    }

    /**
        Returns the newest entry in the array
        @return The most recently added entry
    */
    DataType newest()
    {
        return mData[NumberEntries-1];
    }

    /**
        Returns the oldest entry in the array
        @return The oldest entry placed into the array
    */
    DataType oldest()
    {
        return mData[0];
    }

    /**
        Array access operator. It does do bounds checks.
    */
    DataType operator [] (int index)
    {
        if( index <0 || index >= NumberEntries )
            return mData[0];
        else
            return mData[index];
    }

};

#endif
