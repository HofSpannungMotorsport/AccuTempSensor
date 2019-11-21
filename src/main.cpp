#include <Arduino.h>
#include "Steroido.h"
#include "softReset.h"
#include "TempSensor.h"
#include "SensorData.h"
#include "SoftwareSerial.h"

#include "Pins_V4.h"

#define SOFT_SERIAL_BAUD 28800
#define SERIAL_BUFFER_SIZE 256
#define SEND_INTERVAL    50 // ms
#define RESET_TIME 1000 // ms


SoftwareSerial softSerial(SOFT_SERIAL_RX, SOFT_SERIAL_TX);

TempSensor tempSensors[] = {{ANALOG_SENSOR_0, ENABLE_SENSOR_0},
                            {ANALOG_SENSOR_1, ENABLE_SENSOR_1},
                            {ANALOG_SENSOR_2, ENABLE_SENSOR_2},
                            {ANALOG_SENSOR_3, ENABLE_SENSOR_3},
                            {ANALOG_SENSOR_4, ENABLE_SENSOR_4},
                            {ANALOG_SENSOR_5, ENABLE_SENSOR_5},
                            {ANALOG_SENSOR_6, ENABLE_SENSOR_6},
                            {ANALOG_SENSOR_7, ENABLE_SENSOR_7}};

DigitalIn aliveIn(ALIVE_INPUT);
DigitalOut aliveOut(ALIVE_OUTPUT);

#define SENSOR_COUNT sizeof(tempSensors) / sizeof(TempSensor)
#define DATA_MESSAGE_SIZE 4 + (2 * SENSOR_COUNT) // See SensorData.h for more info

char serialInBuffer[SERIAL_BUFFER_SIZE];
uint8_t boardId;
uint16_t currentMessageSize;
uint16_t sendInterval;
Timer sendTimer;
Timer resetTimer;
bool sendIntervalActive;
bool resetNeeded;


void setup() {
    // Set values here instead of directly after declaring the variable, to be 100% resetNeeded save
    boardId = 0;
    currentMessageSize = 0;
    sendInterval = 0;
    sendIntervalActive = false;
    resetNeeded = false;
    sendTimer.reset();
    resetTimer.reset();
    memSet<char>(serialInBuffer, 0, SERIAL_BUFFER_SIZE);

    softSerial.begin(SOFT_SERIAL_BAUD);

    if (!softSerial.isListening()) {
        softSerial.listen();
    }
}


inline void sendBytes(char* buffer, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        softSerial.write(buffer[i]);
    }
}

inline bool checkSymmetry(char* buffer, uint16_t size) {
    if (size & 1) return false; // -> if (size % 2 == 1) in short

    uint16_t halfSize = size >> 1;

    for (uint16_t i = 0; i < halfSize; i++) {
        if (buffer[i] != ~buffer[i + halfSize])
            return false;
    }

    return true;
}

inline bool checkMessage(char* buffer, uint16_t size) {
    if (buffer[size - 1] != '\n') return false;

    if (*buffer == 'C') {
        return checkSymmetry(buffer + 1, size - 2);
    } else if (*buffer == 'D') {
        SensorData sensorData;
        return sensorData.setByMessage(buffer, size);
    }
    
    return false;
}

inline void activateReset() {
    if (!resetNeeded) {
        resetNeeded =  true;
        resetTimer.start();
    }
}

// Parse a received Message
// Message has to be valid!
inline void parseMessage(char* buffer, uint16_t size) {
    if (*buffer == 'C') {
        uint8_t command = buffer[1];

        if (command == 0) {
            // Nothing
        } else if (command == 1) {
            boardId = buffer[2];
        } else if (command == 2) {
            boardId = buffer[2];
            sendBytes(buffer, size);
        } else if (command == 3) {
            boardId = buffer[2];
            buffer[2]++;
            buffer[4] = ~buffer[2];
            sendBytes(buffer, size);
        } else if (command == 4) {
            char tellBoardId[] = {'C', 5, boardId, ~(4), ~boardId, '\n'};
            sendBytes(tellBoardId, sizeof(tellBoardId) / sizeof(char));
        } else if (command == 5) {
            // Nothing to do
        } else if (command == 6) {
            activateReset();
        } else if (command == 7) {
            activateReset();
            sendBytes(buffer, size);
        }
    }
}

void loop() {
    if (resetNeeded) {
        if (resetTimer.read_ms() > RESET_TIME)
            reset();
    } else {
        if (boardId != 0) {
            if (sendTimer.read_ms() > SEND_INTERVAL) {
                sendTimer.reset();

                SensorData sensorData;
                sensorData.boardId = boardId;

                for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
                    sensorData.temp[i] = tempSensors[i];
                }

                char sendBuffer[DATA_MESSAGE_SIZE];
                sensorData.getMessage(sendBuffer, DATA_MESSAGE_SIZE);

                sendBytes(sendBuffer, DATA_MESSAGE_SIZE);
            }
        }

        while (softSerial.available() > 0) {
            // Read Message
            char currentChar = softSerial.read();
            serialInBuffer[currentMessageSize++] = currentChar;

            if (currentMessageSize >= SERIAL_BUFFER_SIZE) {
                // Prevent Buffer overflow by wrong message
                currentMessageSize = 0;
                return;
            } else if (currentChar == '\n') {
                // To faster pass throu a message from a device before, make special case
                if (serialInBuffer[0] == 'D') {
                    sendBytes(serialInBuffer, currentMessageSize);
                } else {
                    if (checkMessage(serialInBuffer, currentMessageSize)) {
                        parseMessage(serialInBuffer, currentMessageSize);
                    }
                }

                currentMessageSize = 0;
            }
        }
    }
}