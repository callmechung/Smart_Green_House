#include "task_light.h"

static const uint32_t LOG_INTERVAL_MS = 2000;

void task_light(void *pvParamter)
{
    int cur_read[NUM_SECTION];
    int filtered[NUM_SECTION] = {0, 0, 0};
    uint32_t last_log_ms[NUM_SECTION] = {0, 0, 0};

    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].light_pin, INPUT);
        filtered[i] = analogRead(section[i].light_pin);
    }

    while (1)
    {

        for (int i = 0; i < NUM_SECTION; i++)
        {
            analogRead(section[i].light_pin);   
            
            cur_read[i] = analogRead(section[i].light_pin);

            filtered[i] = (filtered[i] * 4 + cur_read[i]) / 5;
        }

        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                section[i].light_raw = filtered[i];
                section[i].light_percent = 100 - (filtered[i] * 100) / 4095;
            }
            xSemaphoreGive(xSensor);
        }

        uint32_t now = millis();
        for (int i = 0; i < NUM_SECTION; i++)
        {
            if (now - last_log_ms[i] >= LOG_INTERVAL_MS)
            {
                Serial.printf("[Light] S%d: raw=%d  light=%d%%\n",
                              i + 1,
                              section[i].light_raw,
                              section[i].light_percent);
                last_log_ms[i] = now;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}