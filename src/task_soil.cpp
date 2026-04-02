#include "task_soil.h"

void task_soil(void *pvParmeter)
{
    int cur_analog_read[NUM_SECTION] = {-1, -1, -1};
    
    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].soil_sensor_pin, INPUT);
        pinMode(section[i].soil_power_pin, OUTPUT);
        digitalWrite(section[i].soil_power_pin, LOW);
    }

    while (1)
    {
        // TAKE VALUE
        for (int i = 0; i < NUM_SECTION; i++)
        {
            digitalWrite(section[i].soil_power_pin, HIGH);
            vTaskDelay(pdMS_TO_TICKS(50)); // wait for stable

            cur_analog_read[i] = analogRead(section[i].soil_sensor_pin);
            digitalWrite(section[i].soil_power_pin, LOW);
        }

        // UPDATE VALUE
        if (xSensor != NULL &&
            xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                section[i].soil_sensor_value = cur_analog_read[i];
            }
            xSemaphoreGive(xSensor);
        }

        for (int i = 0; i < NUM_SECTION; i++)
        {
            if (cur_analog_read[i] <= X_WET)
            {
                Serial.printf("Section %d is EXTREMELY WET!!!\n", i + 1);
            }
            else if (cur_analog_read[i] <= WET)
            {
                Serial.printf("Section %d is WET!!!\n", i + 1);
            }
            else
            {
                Serial.printf("Section %d is DRY!!!\n", i + 1);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10000)); // 10s
    }
}