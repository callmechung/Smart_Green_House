#include "task_light.h"

static const int DEBOUNCE_STABLE = 5;
static const uint32_t LOG_INTERVAL_MS = 5000;

void task_light(void *pvParamter)
{
    int cur_read[NUM_SECTION];
    int pending[NUM_SECTION];
    int confirmed[NUM_SECTION];
    int stable_count[NUM_SECTION];
    uint32_t last_log_ms[NUM_SECTION];

    //  ======== Set up pins and debounce counter ========
    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].light_pin, INPUT);
        cur_read[i] = 0;
        pending[i] = 0;
        confirmed[i] = 0;
        stable_count[i] = 0;
        last_log_ms[i] = 0;
    }

    while (1)
    {
        // ======== Step 1: Read current analog values and apply debouncing ========
        for (int i = 0; i < NUM_SECTION; i++)
        {
            cur_read[i] = analogRead(section[i].light_pin);

            if (cur_read[i] == pending[i])
            {
                if (stable_count[i] < DEBOUNCE_STABLE)
                    stable_count[i]++;
                if (stable_count[i] >= DEBOUNCE_STABLE)
                    confirmed[i] = pending[i];
            }
            else
            {
                pending[i] = cur_read[i];
                stable_count[i] = 1;
            }
        }
        // ======== Step 2: Update global value ========

        if (xSensor != NULL &&
            xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                if (section[i].light_raw != confirmed[i])
                {
                    section[i].light_raw = confirmed[i];
                    section[i].light_percent = 100 - (confirmed[i] * 100) / 4095;
                }
            }
            xSemaphoreGive(xSensor);
        }
        //  ======== Step 3: Log value ========
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