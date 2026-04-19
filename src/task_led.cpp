#include "task_led.h"

void task_led(void *pvParameter)
{
    // Các biến local để lấy data (chống nghẽn Mutex)
    int local_light_percent[NUM_SECTION];
    bool local_auto[NUM_SECTION];
    int local_threshold[NUM_SECTION];
    int local_cmd_brightness[NUM_SECTION]; // Giá trị % từ 0 - 100

    // ======== 1. Cấu hình Pin ban đầu ========
    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].light_ctrl_pin, OUTPUT);
        analogWrite(section[i].light_ctrl_pin, 0); // Tắt đèn lúc khởi động
    }

    while (1)
    {
        // ======== 2. Cập nhật dữ liệu Local từ Global ========
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                local_light_percent[i] = section[i].light_percent;
                local_auto[i] = section[i].is_auto_light;
                local_threshold[i] = section[i].light_threshold;
                local_cmd_brightness[i] = section[i].led_brightness;
            }
            xSemaphoreGive(xSensor);
        }

        // ======== 3. Xử lý Logic Auto / Manual ========
        for (int i = 0; i < NUM_SECTION; i++)
        {
            int final_percent = 0; // Tính toán bằng phần trăm (0-100%)

            // ----- CHẾ ĐỘ AUTO -----
            if (local_auto[i] == true)
            {
                if (local_light_percent[i] <= local_threshold[i])
                {
                    final_percent = 100; // Tối quá -> Bật 100% sáng
                }
                else if (local_light_percent[i] >= local_threshold[i] + 5)
                {
                    final_percent = 0; // Đủ sáng (có bù trừ nhiễu 5%) -> Tắt đèn (0%)
                }
                else
                {
                    final_percent = local_cmd_brightness[i]; // Đang ở vùng nhiễu thì giữ nguyên
                }

                // Cập nhật lại vào Struct nếu có sự thay đổi để Web hiển thị đúng trạng thái
                if (final_percent != local_cmd_brightness[i])
                {
                    if (xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
                    {
                        section[i].led_brightness = final_percent;
                        section[i].is_light_on = (final_percent > 0);
                        xSemaphoreGive(xSensor);
                    }
                }
            }
            // ----- CHẾ ĐỘ MANUAL -----
            else
            {
                final_percent = local_cmd_brightness[i]; // Web bảo bao nhiêu % thì chạy bấy nhiêu
            }

            // ======== 4. Ra lệnh cho Phần cứng (Biến % thành PWM) ========
            // Hàm map: Chuyển thang đo 0-100 thành 0-255 cho analogWrite
            int pwm_value = map(final_percent, 0, 100, 0, 255);

            // Khóa an toàn bổ sung đảm bảo PWM không bao giờ vọt xà
            pwm_value = constrain(pwm_value, 0, 255);

            analogWrite(section[i].light_ctrl_pin, pwm_value);
        }

        // Vòng lặp chạy mỗi 0.5s
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}