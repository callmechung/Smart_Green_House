#include "task_fan.h"

void task_fan(void *pvParameter)
{
    float local_air_temp = 0.0;
    bool local_auto[NUM_SECTION];
    float local_threshold[NUM_SECTION];
    bool local_fan_cmd[NUM_SECTION];

    // ======== 1. Cấu hình Pin ban đầu ========
    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].fan_relay_pin, OUTPUT);
        digitalWrite(section[i].fan_relay_pin, LOW); // Tắt quạt lúc khởi động
    }

    while (1)
    {
        // ======== 2. Lấy Data từ biến Global ========
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            local_air_temp = air_temp; // Nhiệt độ dùng chung cho cả 3 khu (từ DHT22)

            for (int i = 0; i < NUM_SECTION; i++)
            {
                local_auto[i] = section[i].is_auto_fan;
                local_threshold[i] = section[i].temp_threshold;
                local_fan_cmd[i] = section[i].is_fan_on;
            }
            xSemaphoreGive(xSensor);
        }

        // ======== 3. Xử lý Logic Auto / Manual ========
        for (int i = 0; i < NUM_SECTION; i++)
        {
            bool final_fan_state = false;

            if (local_auto[i] == true)
            {
                // ----- CHẾ ĐỘ AUTO -----
                if (local_air_temp >= local_threshold[i])
                {
                    final_fan_state = true; // Nóng quá -> Bật quạt
                }
                else if (local_air_temp <= local_threshold[i] - 1.0)
                {
                    // Mát hơn ngưỡng 1 độ C -> Tắt quạt (Bù trừ nhiễu)
                    final_fan_state = false;
                }
                else
                {
                    // Nằm trong vùng nhiễu thì giữ nguyên trạng thái cũ
                    final_fan_state = local_fan_cmd[i];
                }

                // Cập nhật ngược lại trạng thái vào struct để Web hiển thị
                if (final_fan_state != local_fan_cmd[i])
                {
                    if (xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
                    {
                        section[i].is_fan_on = final_fan_state;
                        xSemaphoreGive(xSensor);
                    }
                }
            }
            else
            {
                // ----- CHẾ ĐỘ MANUAL -----
                final_fan_state = local_fan_cmd[i];
            }

            // ======== 4. Ra lệnh cho Phần cứng ========
            digitalWrite(section[i].fan_relay_pin, final_fan_state ? HIGH : LOW);
        }

        // Quét 1 giây 1 lần
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}