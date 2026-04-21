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
            Serial.println("[Ultrasonic] ❌ Sensor error");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        new_level = TANK_LEVEL - cur_dis;
        new_level = constrain(new_level, 0.0f, (float)TANK_LEVEL);

        //  ======== Step 3: Update Global ========
        if (xSensor != NULL &&
            xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            water_level = new_level;
            xSemaphoreGive(xSensor); // ← trả mutex trước khi print

        }

        
        vTaskDelay(pdMS_TO_TICKS(2000)); // 0.1s
    }
}    