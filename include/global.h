#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

/*
  PIN CONFIGURATION:

  SECTION-LEVEL (3 sections, same pattern repeated):
    Soil Sensor:
      - ADC input:          GPIO 36 (S1), 39 (S2), 34 (S3)
      - Power output:       GPIO 25 (S1), 26 (S2), 27 (S3) => BỎ THAY LÀM FAN CONTROL => FIX LATER
    Light Sensor:
      - ADC input: GPIO:    GPIO 32 (S1), 33 (S2), 35 (S3)
    Pump Relay:
      - Digital output:     GPIO  5 (S1), 18 (S2), 19 (S3)
    LED Control:
      - PWM output:         GPIO 12 (S1), 13 (S2), 14 (S3)

  NODE-LEVEL:
    DHT22: GPIO 4 (Temperature/Humidity)
    Ultrasonic:
      - Trigger: GPIO 16
      - Echo: GPIO 17
*/

#define NUM_SECTION 3

#define DHT_PIN 4
#define DHT_TYPE DHT22

#define ULTRASONIC_TRIG_PIN 16
#define ULTRASONIC_ECHO_PIN 17

/*
  TANK & SOIL CALIBRATION:
  Water tank level: 10 cm (assumed height from sensor to tank bottom)
  Soil moisture thresholds:
    - Extremely Wet : 0-2000 (sensor saturated)
    - Wet           : 2000-3200 (adequate moisture)
    - Dry           : 3200-4095 (needs watering)
*/

#define TANK_LEVEL     10.0
#define SOIL_DRY_VALUE 4095
#define SOIL_WET_VALUE 1500

struct GARDEN_SECTION
{
  // Identifers & INPUT PINs
  int id;
  int soil_sensor_pin;
  int light_pin;

  // OUTPUT PINs
  int pump_pin;
  int led_pin;
  int fan_relay_pin;

  // Sensor raw data - 12 bits ADC
  int soil_raw;     
  int light_raw;

  int soil_percent;   // 0% = VERY DRY | 100% EXTREMELY WET
  int light_percent;  // 0% = DARK     | 100% = BRIGHT

  // Devices State
  bool is_pump_on;
  bool is_led_on;
  bool is_fan_on;

  // Flag mode
  bool is_auto_pump;
  bool is_auto_led;
  bool is_auto_fan;

  // Threshold & value
  int led_brightness;

  int soil_wet_threshold;   // after conver to percent
  int soil_dry_threshold;   // after conver to percent
  int light_threshold;
  float temp_threshold;
};

// ========== GLOBAL STATE VARIABLES ==========

extern GARDEN_SECTION section[NUM_SECTION];

extern float air_temp;
extern float air_humid;
extern float water_level; 


extern SemaphoreHandle_t xSensor;
extern bool isWifiConnected;
extern bool isMqttConnected;

#endif