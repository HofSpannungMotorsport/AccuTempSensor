#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include "TempSensorValues.h"

#define SHORT_CIRCUIT_VALUE -128
#define DISCONNECTED_VALUE   127
#define TEMP_ERROR_OTHER    -127

#define SMALLEST_VALUE -41
#define HIGHEST_VALUE  121

#define SHORT_CIRCUIT_THRESHHOLD 64879 // uint16_t ranges from 0 to 65535
#define DISCONNECTED_THRESHHOLD 655    // Applying 1% margin

#define SHORT_CIRCUIT_TIMEOUT 100 // ms
#define DISCONNECTED_TIMEOUT  100 // ms

class TempSensor : NonCopyable<TempSensor> {
    public:
        TempSensor(PinName analogIn, PinName enableOut)
        : _analog(analogIn), _enable(enableOut) {}

        void reset() {
            _shortCircuitFlag = false;
            _disconnectFlag = false;

            _shortCircuitStarted = false;
            _disconnectStarted = false;
        }

        void preActivate() {
            _enable = 1;
        }

        void deActivate() {
            _enable = 0;
        }
    
        int8_t read() {
            // Get Value (by activating the enable pin)
            preActivate();
            uint16_t analogValue = _analog.read_u16();
            deActivate();

            // Check Boundary
            _shortCircuitFlag |= checkBoundary(analogValue > SHORT_CIRCUIT_THRESHHOLD, _shortCircuitStarted, _shortCircuitTime, SHORT_CIRCUIT_TIMEOUT);
            _disconnectFlag |= checkBoundary(analogValue < DISCONNECTED_THRESHHOLD, _disconnectStarted, _disconnectTime, DISCONNECTED_TIMEOUT);

            if (_disconnectFlag) {
                return DISCONNECTED_VALUE;
            }

            if (_shortCircuitFlag) {
                return SHORT_CIRCUIT_VALUE;
            }

            if (_shortCircuitStarted || _disconnectStarted) {
                return _lastValidReading;
            }

            // Get Temperature
            int8_t currentTemp = TEMP_ERROR_OTHER;

            if (analogValue > tempConversion[0][0]) {
                currentTemp = SMALLEST_VALUE;
            } else {
                uint8_t i = 0;
                while (true) {
                    if (tempConversion[i + 1][0] == -1) {
                        currentTemp = HIGHEST_VALUE;
                        break;
                    }
                    
                    if (analogValue < tempConversion[i][0] && analogValue > tempConversion[i + 1][0]) {
                        currentTemp = mapC<int16_t>(analogValue, tempConversion[i][0], tempConversion[i + 1][0], tempConversion[i][1], tempConversion[i + 1][1]);
                        break;
                    }

                    ++i;
                }
            }

            _lastValidReading = currentTemp;
            return currentTemp;
        }

        operator int8_t() {
            return read();
        }

    private:
        AnalogIn _analog;
        DigitalOut _enable;

        bool _shortCircuitStarted = false;
        bool _disconnectStarted = false;

        bool _shortCircuitFlag = false;
        bool _disconnectFlag = false;

        Timer _shortCircuitTime;
        Timer _disconnectTime;

        int8_t _lastValidReading = TEMP_ERROR_OTHER;

        inline bool checkBoundary(bool condition, bool &started, Timer &timer, unsigned long timeout) {
            if (condition) {
                if (started) {
                    if (timer.read_ms() > timeout) {
                        return true;
                    }
                } else {
                    timer.reset();
                    timer.start();
                    started = true;
                }
            } else {
                started = false;
            }

            return false;
        }
};

#endif // TEMP_SENOSR_H