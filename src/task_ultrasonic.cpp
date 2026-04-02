#include "task_ultrasonic.h"

void task_ultrasonic(void *pvParemeter)
{
    //  ======== Initialize HCSR05 and local variables ========
    HCSR04 hcsr04(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);

    float cur_dis = -1.0;  // Current distance reading
    float new_level = 0.0; // Calculated water level

    while (1)
    {
        //  ======== Step 1: Read distance from sensor ========
        cur_dis = hcsr04.dist();

        //  ======== Step 2: Check validiation & Update ========
        if (isnan(cur_dis) || cur_dis < 0)
        {
            Serial.printf("❌ ULTRASONIC SENSOR ERROR!!!\n");
            Serial.printf("   Reading: %s (expected 0-%.1f cm)\n",
                          isnan(cur_dis) ? "NaN" : String(cur_dis).c_str(),
                          TANK_LEVEL);
        }
        else
        {
            // Sensor OK
            new_level = TANK_LEVEL - cur_dis;
            // valid range = [0, TANK_LEVEL]

            if (new_level < 0.0)
            {
                Serial.println("WARNING: WATER LEVEL NEGATIVE");
                new_level = 0;
            }
            else if (new_level > TANK_LEVEL)
            {
                Serial.println("WARNING: WATER LEVEL OVER TANK LEVEL");
                new_level = TANK_LEVEL;
            }

            Serial.printf("Ultrasonic OK | Distance: %.1f cm | Water Level: %.1f cm\n",
                          cur_dis, new_level);

            if (xSensor != NULL &&
                xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
            {
                water_level = new_level;
                xSemaphoreGive(xSensor);
            }
        }
        

        vTaskDelay(pdMS_TO_TICKS(2000)); // 0.1s
    }
}