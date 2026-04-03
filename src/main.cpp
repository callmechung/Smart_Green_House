#include "global.h"

#include "task_WIFI.h"
#include "task_mqtt.h"

#include "task_dht.h"
#include "task_light.h"
#include "task_soil.h"
#include "task_ultrasonic.h"

void setup()
{
  Serial.begin(115200);
  xSensor = xSemaphoreCreateMutex();

  Serial.println("Set up begin");

  xTaskCreate(task_wifi, "TASK WIFI", 4096, NULL, 2, NULL);
  xTaskCreate(task_mqtt, "TASK MQTT", 4096, NULL, 2, NULL);
  xTaskCreate(task_dht, "TASK DHT", 2048, NULL, 1, NULL);
  xTaskCreate(task_ultrasonic, "TASK ULTRASONIC", 2048, NULL, 1, NULL);
  xTaskCreate(task_soil, "TASK_SOIL", 2048, NULL, 1, NULL);
  xTaskCreate(task_light, "TASK_LIGHT", 2048, NULL, 1, NULL);
}

void loop() {}