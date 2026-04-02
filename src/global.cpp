#include "global.h"


GARDEN_SECTION section[NUM_SECTION] = {
    // {id, soil_sensor_pin (ADC1), soil_power_pin (Output), light_pin (ADC1), sensor_value, light}
    {1, 36, 25, 32, 0.0, false}, 
    {2, 39, 26, 33, 0.0, false}, 
    {3, 34, 27, 35, 0.0, false}, 
};

float air_temp = 0.0;
float air_humid = 0.0;
float water_level = 0.0;

SemaphoreHandle_t xSensor = NULL;
bool isWifiConnected = false;