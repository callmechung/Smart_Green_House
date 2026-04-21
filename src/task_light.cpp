#include "task_light.h"

static const uint32_t LOG_INTERVAL_MS = 2000;

void task_light(void *pvParamter)
{
    int cur_read[NUM_SECTION];
    int filtered[NUM_SECTION] = {0, 0, 0};
    uint32_t last_log_ms[NUM_SECTION] = {0, 0, 0};

    int light_threshold[NUM_SECTION]; 

    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].light_pin, INPUT);
        filtered[i] = analogRead(section[i].light_pin);
    }

    while (1)
    {
        // ======== Step 0: Update threshold ========
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                light_threshold[i] = section[i].light_threshold;
            }
            xSemaphoreGive(xSensor);
        }

        // ======== Step 1: Read all ADC value ========
        for (int i = 0; i < NUM_SECTION; i++)
        {
            analogRead(section[i].light_pin);
            cur_read[i] = analogRead(section[i].light_pin);
            filtered[i] = (filtered[i] * 4 + cur_read[i]) / 5;
        }

        // ======== Step 2: Update global values ========
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                section[i].light_raw = filtered[i];
                section[i].light_percent = 100 - (filtered[i] * 100) / 4095;
            }
            xSemaphoreGive(xSensor);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}