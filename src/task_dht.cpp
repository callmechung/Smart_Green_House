#include "task_dht.h"

void task_dht(void *pvParameter)
{
    // Set up the sensor
    DHT dht22(DHT_PIN, DHT_TYPE);
    dht22.begin();
    float cur_t = -1;
    float cur_h = -1;

    while (1)
    {
        cur_h = dht22.readHumidity();
        cur_t = dht22.readTemperature();

        if (isnan(cur_h) || isnan(cur_t))
        {
            Serial.println("Sensor is DISCONNECTED!!!");
        }
        else
        {
            Serial.printf("Temperature: %.1f C\n", cur_t);
            Serial.printf("Humidity: %.1f %%\n", cur_h);
        }

        if (xSensor != NULL &&
            xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            air_temp = cur_t;
            air_humid = cur_h;
            xSemaphoreGive(xSensor);
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Read only 2sec
    }
}