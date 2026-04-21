#include "task_dht.h"

void task_dht(void *pvParameter)
{
    //  ======== Initialize DHT22 and local variables ========
    DHT dht22(DHT_PIN, DHT_TYPE);
    dht22.begin();

    float cur_t = 0.0;
    float cur_h = 0.0;
    //bool sensor_valid = false;

    while (1)
    {
        //  ======== Step 1: Read value from DHT22 ========
        cur_h = dht22.readHumidity();
        cur_t = dht22.readTemperature();

        //  ======== Step 2: Check validiation & Update ========
        if (isnan(cur_h) || isnan(cur_t))
        {
            Serial.println("[DHT] ❌ Sensor error (NaN)");
        }
        else
        {
            // Update global variables with valid data
            if (xSensor != NULL &&
                xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
            {
                air_temp = cur_t;
                air_humid = cur_h;
                xSemaphoreGive(xSensor);
            }       
            // Log value
            //Serial.printf("[DHT] Temp: %.1f°C  Humid: %.1f%%\n", cur_t, cur_h);
        }
        

        vTaskDelay(pdMS_TO_TICKS(2000)); // Read only 2sec
    }
}