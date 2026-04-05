#include "global.h"

GARDEN_SECTION section[NUM_SECTION] =
{
    //  id  soil_sens  light_in  pump_relay  led_pwm  raw  raw  %    %    pump   led    brightness
    {1, 36, 32, 5, 21, 0, 0, 0, 0, false, false, 0},  // S1 — LED: 12→21
    {2, 39, 33, 18, 13, 0, 0, 0, 0, false, false, 0}, // S2 — pump: 6→18
    {3, 34, 35, 19, 14, 0, 0, 0, 0, false, false, 0}, // S3 — pump: 7→19
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