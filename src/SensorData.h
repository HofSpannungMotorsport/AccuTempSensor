#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

/*
    The Sensor-Data Class is ment to be a data container for a single
    Temp Sensor Reading including all Sensors on the Board (8).

    It also comes with some methods to convert messages received
    over Serial or to send them over Serial

    A single Message looks like this:
    1 Char (byte) Message Type (C = Config, D = Data Message)
    
    For Config:
    Shown in a different File

    For Data:
    1 Byte Board ID
    X Byte Sensor Data
    1 Byte Board ID (to be save, inverted)
    X Byte Sensor Data (to be save, inverted)
    1 Char (byte) ending (\n)
*/

#define SENSOR_COUNT_PER_BOARD 8
#define SENSOR_DATA_MESSAGE_LENGTH ((SENSOR_COUNT_PER_BOARD * 2) + 4)

class SensorData {
    public:
        uint8_t boardId = 0;
        int8_t temp[SENSOR_COUNT_PER_BOARD];

        /*
            Copy the Data from a Message into a SensorData Instance

            @param message The pointer to the buffer array with the data of the message
            @param length The length of the message (should be constant)
            @return bool true if the Message was copyed, false if the given Message is invalid
        */
        bool setByMessage(int8_t* message, uint16_t length) {
            if (length != SENSOR_DATA_MESSAGE_LENGTH) return false;
            if (*message != 'D') return false; // Check Beginning
            if (message[SENSOR_DATA_MESSAGE_LENGTH - 1] != '\n') return false; // Check ending

            uint8_t currentBoardId = message[1];
            uint8_t currentBoardIdInverted = message[1 + SENSOR_COUNT_PER_BOARD];
            
            if (currentBoardId != ~currentBoardIdInverted) return false;

            int8_t currentData[SENSOR_COUNT_PER_BOARD];

            // Check Data
            for (uint8_t i = 0; i < SENSOR_COUNT_PER_BOARD; i++) {
                currentData[i] = message[i + 2];
                if (currentData[i] != ~message[i + 2 + 1 + SENSOR_COUNT_PER_BOARD]) return false;
            }

            // Copy Data
            boardId = currentBoardId;
            memCpy<int8_t>(temp, currentData, SENSOR_COUNT_PER_BOARD);

            return true;
        }

        /*
            Create a Message to be sent over serial (or some other byte-wise-stream)
            Including correction data.

            @param messageBuffer The buffer the message should be written to
            @param length The size of the buffer in elements
            @return bool true if the message was successfully build
        */
        bool getMessage(char* messageBuffer, uint16_t length) {
            if (length < SENSOR_DATA_MESSAGE_LENGTH) return false;

            *messageBuffer = 'D'; // for Data

            // Set board-id
            messageBuffer[1] = boardId;
            messageBuffer[1 + SENSOR_COUNT_PER_BOARD] = ~boardId;

            // Write Data
            for (uint8_t i = 0; i < SENSOR_COUNT_PER_BOARD; i++) {
                messageBuffer[i + 2] = temp[i];
                messageBuffer[i + 2 + 1 + SENSOR_COUNT_PER_BOARD] = ~temp[i];
            }

            // Add ending
            messageBuffer[SENSOR_DATA_MESSAGE_LENGTH - 1] = '\n';

            return true;
        }
};

#endif // SENSOR_DATA_H