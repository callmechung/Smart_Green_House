#include "task_fan.h"

static bool is_on[NUM_SECTION];
static bool is_auto[NUM_SECTION];
static float temp_th[NUM_SECTION];  // Thershold riêng cho từng vùng
static float cur_temp;              // Cả 3 section được đo từ cùng 1 nguồn nhiệt độ

void task_fan(void *pvParameter)
{
    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].fan_relay_pin, OUTPUT);
        digitalWrite(section[i].fan_relay_pin, LOW);
    }

    while (1)
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            cur_temp = air_temp;

            for (int i = 0; i < NUM_SECTION; i++)
            {
                is_on[i] = section[i].is_fan_on;
                is_auto[i] = section[i].is_auto_fan;
                temp_th[i] = section[i].temp_threshold;
            }

            xSemaphoreGive(xSensor);
        }

        for (int i = 0; i < NUM_SECTION; i++)
        {
            // ===== MANUAL MODE ====
            if (is_auto[i] == false)
            {
                // --- command ON ---
                if (is_on[i] == true)
                    digitalWrite(section[i].fan_relay_pin, HIGH);
                else
                    digitalWrite(section[i].fan_relay_pin, LOW);
            }

            // ===== AUTO MODE =====
            else
            {
                // Khi nhiệt độ hiện tại lớn hơn ngưỡng của vùng
                if (cur_temp >= temp_th[i])
                {
                    digitalWrite(section[i].fan_relay_pin, HIGH);
                    is_on[i] = true;
                }
                else if (cur_temp <= (temp_th[i] - 1.0))
                {
                    // Mát hơn ngưỡng ít nhất 1 độ => Tắt quạt
                    digitalWrite(section[i].fan_relay_pin, LOW);
                    is_on[i] = false;
                }
                else
                {
                    // Nằm giữa vùng nhiễu (VD: 34.1 đến 34.9) => Giữ nguyên trạng thái cũ
                    if (is_on[i] == true)
                        digitalWrite(section[i].fan_relay_pin, HIGH);
                    else
                        digitalWrite(section[i].fan_relay_pin, LOW);
                }
            }

            if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
            {
                section[i].is_fan_on = is_on[i];
                xSemaphoreGive(xSensor);
            }

        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}