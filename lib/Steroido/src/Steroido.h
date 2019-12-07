#ifndef STEROIDO_H
#define STEROIDO_H

#include "Common/freeMemory.h"
#include "Common/memCpy.h"
#include "Common/memSet.h"
#include "Common/mapC.h"


// Include Arduino Hardware Drivers
#ifdef Arduino_h
    // STL
    #include "Common/vector.h"

    // Abstraction Layer
    #include "AbstractionLayer/Arduino/printfIntegration.h"
    #include "AbstractionLayer/Arduino/NonCopyable.h"
    #include "AbstractionLayer/Arduino/Callback.h"
    #include "AbstractionLayer/Arduino/CircularBuffer.h"
    #include "AbstractionLayer/Arduino/Timer.h"
    #include "AbstractionLayer/Arduino/PinName.h"
    #include "AbstractionLayer/Arduino/PinMode.h"
    #include "AbstractionLayer/Arduino/AnalogIn.h"
    #include "AbstractionLayer/Arduino/AnalogOut.h"
    #include "AbstractionLayer/Arduino/DigitalIn.h"
    #include "AbstractionLayer/Arduino/DigitalOut.h"
    #include "AbstractionLayer/Arduino/PwmOut.h"
#endif // Arduino_h


#ifdef TEENSY
    #include "AbstractionLayer/Teensyduino/CAN.h"
#endif // TEENSY


#ifdef MBED_H
    // Include nothing, brings all with it
    #define VECTOR_EMPLACE_BACK_ENABLED
#endif // MBED_H


#include "Common/DelayedSwitch.h"


#endif // STEROIDO_H