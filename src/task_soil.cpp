#include "task_soil.h"

void task_soil(void *pvParmeter)
{
    int cur_analog_read[NUM_SECTION] = {0, 0, 0};

    //  ======== Set up pins ========
    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].soil_sensor_pin, INPUT);     // ADC input
        pinMode(section[i].soil_power_pin, OUTPUT);     // Power control
        digitalWrite(section[i].soil_power_pin, LOW);   // Keep OFF
    }

    while (1)
    {
        // ======== Step 1: Read all ADC value of soil moister sensor ========
        for (int i = 0; i < NUM_SECTION; i++)
        {
            // Enable sensor
            digitalWrite(section[i].soil_power_pin, HIGH);

            // wait for stable
            vTaskDelay(pdMS_TO_TICKS(50));  

            // Read 12-bit ADC value (0-4095)
            cur_analog_read[i] = analogRead(section[i].soil_sensor_pin);
            
            // Diasble sensor
            digitalWrite(section[i].soil_power_pin, LOW);
        }

        // ======== Step 2: Update global values ========
        if (xSensor != NULL &&
            xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                // Store raw ADC value;
                section[i].soil_raw = cur_analog_read[i];

                /*Calibaration
                    SOIL_WET_VALUE (1500)  = 100% moisture (saturated)
                    SOIL_DRY_VALUE (4095)  = 0% moisture (completely dry)
                    Formula: percentage = (dry_reading - current_reading) / (dry_reading - wet_reading) * 100
                */

                int percent;
                if (cur_analog_read[i] >= SOIL_DRY_VALUE)
                {
                    // Drier than calibration point - clamp to 0%
                    percent = 0;
                }
                else if (cur_analog_read[i] <= SOIL_WET_VALUE)
                {
                    // Wetter than calibration point - clamp to 100%
                    percent = 100;
                }
                else
                {
                    // Linear interpolation between calibration points
                    percent = 100 - ((cur_analog_read[i] - SOIL_WET_VALUE) * 100) /
                                        (SOIL_DRY_VALUE - SOIL_WET_VALUE);
                }

                section[i].soil_percent = percent;
            }

            // ======== Step 3: Log for debugging ========
            for (int i = 0; i < NUM_SECTION; i++)
            {
                Serial.printf("Section %d: Soil Raw=%d, Moisture=%d%%  ",
                              i + 1,
                              cur_analog_read[i],
                              section[i].soil_percent);

                // Also print moisture category for quick assessment
                if (cur_analog_read[i] <= X_WET)
                {
                    Serial.println("Status: EXTREMELY WET");
                }
                else if (cur_analog_read[i] <= WET)
                {
                    Serial.println("Status: WET");
                }
                else
                {
                    Serial.println("Status: DRY");
                }
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