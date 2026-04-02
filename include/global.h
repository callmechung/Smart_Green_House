#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

/*
  SOIL SENSOR PIN       
  SOIL SENSOR POWER    
  LIGHT PIN             
  DHT_PIN               4
  ULTRASONIC PIN        16, (trig)       17 (echo)
*/

#define NUM_SECTION 3

#define DHT_PIN 4
#define DHT_TYPE DHT22

#define ULTRASONIC_TRIG_PIN 16
#define ULTRASONIC_ECHO_PIN 17

/*
ASSUMPTION    Water tank level    10
              Soil moister level  0 -> 2000 : Extrmely Wet
                                  2000 -> 3200 : Wet
                                  3200 -> 4095 : Dry
*/
#define TANK_LEVEL  10    
#define X_WET       2000
#define WET         3200


struct GARDEN_SECTION 
{
  int id;
  int soil_sensor_pin;
  int soil_power_pin;
  int light_pin;

  float soil_sensor_value;
  bool light;
};

extern GARDEN_SECTION section[NUM_SECTION];
extern float air_temp;
extern float air_humid;
extern float water_level; // đo bằng ultrasonic

extern SemaphoreHandle_t xSensor;
extern bool isWifiConnected;

#endif