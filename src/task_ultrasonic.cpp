#include "task_ultrasonic.h"

void task_ultrasonic(void *pvParemeter)
{
    HCSR04 hcsr04(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);

    float cur_dis = -1;

    while (1)
    {
        cur_dis = hcsr04.dist();

        if (isnan(cur_dis) || cur_dis < 0)
        {
            Serial.printf("ULTRASONIC IS ERROR!!!\n");
        }
        else
        {
            Serial.printf("Current distance: %.1f\n", cur_dis);
            Serial.printf("Water level: %.1f\n", TANK_LEVEL - cur_dis);
        }

        if (xSensor != NULL &&
            xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            water_level = TANK_LEVEL - cur_dis;
            xSemaphoreGive(xSensor);
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // 0.1s
    }
}