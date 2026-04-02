#include "task_light.h"

const int debounce_times = 5;

void task_light(void *pvParamter)
{
    int cur_analog_read[NUM_SECTION];
    int last_analog_read[NUM_SECTION];
    int count[NUM_SECTION];

    //  ======== Set up pins and debounce counter ========
    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].light_pin, INPUT);
        cur_analog_read[i] = 0;
        last_analog_read[i] = 0;
        count[i] = debounce_times;
    }

    while (1)
    {
        // ======== Step 1: Read current analog values and apply debouncing ========
        for (int i = 0; i < NUM_SECTION; i++)
        {
            // Read 12-bit ADC value (0-4095)
            cur_analog_read[i] = analogRead(section[i].light_pin);

            // Check for changing
            if (last_analog_read[i] != cur_analog_read[i])
            {
                // Value changed - decrease debounce counter
                count[i]--;

                // If debounce reached 0, change confirmed
                if (count[i] <= 0)
                {
                    last_analog_read[i] = cur_analog_read[i];
                }
            }
            else
            {  
                // Value stable | Noise -> reset counter
                count[i] = debounce_times;
            }
        }

        // ======== Step 2: Update global value ========

        if (xSensor != NULL &&
            xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                if (section[i].light_raw != last_analog_read[i])
                {
                    // Store raw ADC value (0-4095)
                    section[i].light_raw = last_analog_read[i];

                    // Store percentage
                    section[i].light_percent = (last_analog_read[i] / 4095) * 100;

                    // Reseet debounce counter
                    count[i] = debounce_times;

                    Serial.printf("Section %d: Light Raw=%d, Percent=%d%%\n",
                                  i + 1,
                                  section[i].light_raw,
                                  section[i].light_percent);
                }
            }

            xSemaphoreGive(xSensor);
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // debounce 0.5s
    }
}