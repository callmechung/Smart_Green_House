#include "task_led.h"

static bool is_on[NUM_SECTION];
static bool is_auto[NUM_SECTION];
static int light_th[NUM_SECTION];
static int cur_light[NUM_SECTION];
static int led_lv[NUM_SECTION];

void task_led(void *pvParameter)
{
    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].led_pin, OUTPUT);
        analogWrite(section[i].led_pin, 0);
    }

    while (1)
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                is_on[i] = section[i].is_led_on;
                is_auto[i] = section[i].is_auto_led;
                light_th[i] = section[i].light_threshold;
                cur_light[i] = section[i].light_percent;
                led_lv[i] = section[i].led_brightness;
            }
            xSemaphoreGive(xSensor);
        }

        for (int i = 0; i < NUM_SECTION; i++)
        {
            // ===== MANUAL MODE ====
            if (is_auto[i] == false)
            {
                // PWM != 0
                if (is_on[i] == true)
                {
                    analogWrite(section[i].led_pin, map(led_lv[i], 0, 100, 0, 255));
                }

                // led_lv = 0 handle at mqtt when led_brightness <= 0
                else
                    analogWrite(section[i].led_pin, 0);
            }
            
            // ===== AUTO MODE ====
            else
            {
                if (cur_light[i] <= light_th[i])
                {
                    is_on[i] = true;
                    led_lv[i] = light_th[i] + (100 - light_th[i])/2;

                    analogWrite(section[i].led_pin, map(led_lv[i], 0, 100, 0, 255));
                }
                else
                {
                    is_on[i] = false;
                    analogWrite(section[i].led_pin, 0);
                }
            }

            if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
            {
                section[i].led_brightness = led_lv[i];
                section[i].is_auto_led = is_auto[i];
                section[i].is_led_on = is_on[i];

                xSemaphoreGive(xSensor);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}