#include "global.h"

GARDEN_SECTION section[NUM_SECTION] = 
{
    {1, 36, 25, 32, 5, 12, 0, 0, 0, 0, false, false, 0},
    {2, 39, 26, 33, 6, 13, 0, 0, 0, 0, false, false, 0},
    {3, 34, 27, 35, 7, 14, 0, 0, 0, 0, false, false, 0},
};

float air_temp = 0.0;
float air_humid = 0.0;
float water_level = 0.0;

bool pump_on = false;
bool led_on = false;
int led_pwm = 0;            

SemaphoreHandle_t xSensor = NULL;
bool isWifiConnected = false;
bool isMqttConnected = false;