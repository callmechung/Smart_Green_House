#include "task_pump.h"

static bool is_pump[NUM_SECTION];
static bool auto_flag[NUM_SECTION];
static int wet_threshold[NUM_SECTION];
static int dry_threshold[NUM_SECTION];
static int cur_soil_val[NUM_SECTION];

bool pump_on(int sec)
{
    digitalWrite(section[sec].pump_pin, HIGH);
    return true;
}

bool pump_off(int sec)
{
    digitalWrite(section[sec].pump_pin, LOW);
    return false;
}

void task_pump(void *pvParameter)
{

    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].pump_pin, OUTPUT);
        digitalWrite(section[i].pump_pin, LOW);
    }

    while (1)
    {
        if (xSensor != NULL &&
            xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                is_pump[i] = section[i].is_pump_on;
                auto_flag[i] = section[i].is_auto_pump;
                wet_threshold[i] = section[i].soil_wet_threshold;
                dry_threshold[i] = section[i].soil_dry_threshold;
                cur_soil_val[i] = section[i].soil_percent;
            }

            xSemaphoreGive(xSensor);
        }

        for (int i = 0; i < NUM_SECTION; i++)
        {
            // ===== MANUAL MODE =====
            if (auto_flag[i] == false)
            {
                if (is_pump[i] == true) // => update water tank level later
                {
                    is_pump[i] = pump_on(i);
                }
                else
                {
                    is_pump[i] = pump_off(i);
                }

                if (cur_soil_val[i] >= wet_threshold[i])
                {
                    is_pump[i] = pump_off(i);
                }
            }

            // ===== AUTO MODE =====
            else
            {
                if (cur_soil_val[i] <= dry_threshold[i])
                {
                    is_pump[i] = pump_on(i);
                }
                else if (cur_soil_val[i] >= wet_threshold[i])
                {
                    is_pump[i] = pump_off(i);
                }
                else
                {
                    // Nằm giữa Dry và Wet -> Giữ trạng thái cũ
                    if (is_pump[i])
                        is_pump[i] = pump_on(i);
                    else
                        is_pump[i] = pump_off(i);
                }
            }

            // ===== UPDATE is_pump_on =====
            if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
            {
                section[i].is_pump_on = is_pump[i];
                xSemaphoreGive(xSensor);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}