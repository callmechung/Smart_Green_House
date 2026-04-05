#include "task_soil.h"

void task_soil(void *pvParmeter)
{
    int cur_analog_read[NUM_SECTION] = {0, 0, 0};
    int cur_percent[NUM_SECTION] = {0, 0, 0};

    //  ======== Set up pins ========
    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].soil_sensor_pin, INPUT);     // ADC input
    }

    while (1)
    {
        // ======== Step 1: Read all ADC value of soil moister sensor ========
        for (int i = 0; i < NUM_SECTION; i++)
        {
            // Read 12-bit ADC value (0-4095)
            cur_analog_read[i] = analogRead(section[i].soil_sensor_pin);
        }

        // ======== Step 2: Calculate value ========
        for (int i = 0; i < NUM_SECTION; i++)
        {
            if (cur_analog_read[i] >= SOIL_DRY_VALUE)
                cur_percent[i] = 0;
            else if (cur_analog_read[i] <= SOIL_WET_VALUE)
                cur_percent[i] = 100;
            else
                cur_percent[i] = 100 - ((cur_analog_read[i] - SOIL_WET_VALUE) * 100) / (SOIL_DRY_VALUE - SOIL_WET_VALUE);
        }

        // ======== Step 3: Update global values ========
        if (xSensor != NULL &&
            xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                section[i].soil_raw = cur_analog_read[i];
                section[i].soil_percent = cur_percent[i];
            }

            xSemaphoreGive(xSensor);
        }

        // ======== Step 4: Update global values ========
        for (int i = 0; i < NUM_SECTION; i++)
        {
            const char *status = (cur_analog_read[i] <= X_WET) ? "X-WET"
                                 : (cur_analog_read[i] <= WET) ? "WET"
                                                               : "DRY";
            Serial.printf("[Soil] S%d: raw=%d  moisture=%d%%  [%s]\n",
                          i + 1, cur_analog_read[i], cur_percent[i], status);
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // 2s
    }
}