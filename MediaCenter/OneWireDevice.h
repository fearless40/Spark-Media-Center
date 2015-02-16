#ifndef __ONEWIREDEVICE__
#define __ONEWIREDEVICE__


class OneWireDevice
{
protected:

    uint8_t rom;
    int     lastAccess;

public:

    virtual void update();

}





#endif
