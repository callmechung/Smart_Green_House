#include "global.h"

GARDEN_SECTION section[NUM_SECTION] =
    {
        // Section 1
        {
            1,                   // Identifer
            36, 32,              // Input pins
            5, 21, 25,           // Output pins
            0, 0, 0, 0,          // Sensors raw data - 12 bit & percentage conversion
            false, false, false, // Devices State - default OFF
            true, true, true,    // Flag mode - default AUTO mode
            0,                   // Led brightness
            70, 40, 50, 25.5     // Thresholds
        },

        // Section 2
        {
            2,                   // Identifer
            39, 33,              // Input pins
            18, 13, 26,          // Output pins
            0, 0, 0, 0,          // Sensors raw data - 12 bit & percentage conversion
            false, false, false, // Devices State - default OFF
            true, true, true,    // Flag mode - default AUTO mode
            0,                   // Led brightness
            70, 40, 60, 25.5     // Thresholds
        },

        // Section 3
        {
            3,                   // Identifer
            34, 35,              // Input pins
            19, 14, 27,          // Output pins
            0, 0, 0, 0,          // Sensors raw data - 12 bit & percentage conversion
            false, false, false, // Devices State - default OFF
            true, true, true,    // Flag mode - default AUTO mode
            0,                   // Led brightness
            70, 40, 60, 25.5     // Thresholds
        },
};

float air_temp = 0.0;
float air_humid = 0.0;
float water_level = 0.0;

SemaphoreHandle_t xSensor = NULL;
bool isWifiConnected = false;
bool isMqttConnected = false;