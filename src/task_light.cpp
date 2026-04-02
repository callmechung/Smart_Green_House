#include "task_light.h"

const int debounce_times = 5;
void task_light(void *pvParamter)
{
    int cur_analog_read[NUM_SECTION], last_analog_read[NUM_SECTION], count[NUM_SECTION];

    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].light_pin, INPUT);
        cur_analog_read[i] = 0;
        last_analog_read[i] = 0;
        count[i] = debounce_times;
    }

    while (1)
    {
        for (int i = 0; i < NUM_SECTION; i++)
        {
            cur_analog_read[i] = analogRead(section[i].light_pin);
            if (last_analog_read[i] != cur_analog_read[i])
            {
                if (count[i] <= 0)
                {
                    last_analog_read[i] = cur_analog_read[i];
                }
                count[i]--;
            }
            else
            {
                count[i] = debounce_times;
            }
        }

        if (xSensor != NULL &&
            xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                if (section[i].light == last_analog_read[i])
                    continue;
                else
                {
                    section[i].light = last_analog_read[i];
                    count[i] = debounce_times; // just4sure
                }
            }

            xSemaphoreGive(xSensor);
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // debounce 0.5s
    }
}