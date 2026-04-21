#include "task_monitor.h"

void task_monitor(void *pvParameter)
{
    // Các mảng và biến Local để chứa bản sao (Snapshot) của dữ liệu
    float loc_temp = 0.0, loc_humid = 0.0, loc_water = 0.0;

    int loc_soil[NUM_SECTION];
    int loc_dry_th[NUM_SECTION], loc_wet_th[NUM_SECTION];
    int loc_light[NUM_SECTION], loc_light_th[NUM_SECTION];
    float loc_temp_th[NUM_SECTION];

    bool loc_pump_on[NUM_SECTION], loc_auto_pump[NUM_SECTION];
    bool loc_fan_on[NUM_SECTION], loc_auto_fan[NUM_SECTION];
    bool loc_led_on[NUM_SECTION], loc_auto_led[NUM_SECTION];
    int loc_led_br[NUM_SECTION];

    while (1)
    {
        // ======== 1. LẤY DỮ LIỆU (TAKE MUTEX) ========
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            // Copy biến Global
            loc_temp = air_temp;
            loc_humid = air_humid;
            loc_water = water_level;

            // Copy mảng Section
            for (int i = 0; i < NUM_SECTION; i++)
            {
                loc_soil[i] = section[i].soil_percent;
                loc_dry_th[i] = section[i].soil_dry_threshold;
                loc_wet_th[i] = section[i].soil_wet_threshold;

                loc_light[i] = section[i].light_percent;
                loc_light_th[i] = section[i].light_threshold;

                loc_temp_th[i] = section[i].temp_threshold;

                loc_pump_on[i] = section[i].is_pump_on;
                loc_auto_pump[i] = section[i].is_auto_pump;

                loc_fan_on[i] = section[i].is_fan_on;
                loc_auto_fan[i] = section[i].is_auto_fan;

                loc_led_on[i] = section[i].is_led_on;
                loc_auto_led[i] = section[i].is_auto_led;
                loc_led_br[i] = section[i].led_brightness;
            }

            // 🛡️ TRẢ LẠI MUTEX NGAY LẬP TỨC CHO CÁC TASK KHÁC CHẠY
            xSemaphoreGive(xSensor);
        }

        // ======== 2. IN RA MÀN HÌNH (KHI KHÔNG CÒN CẦM MUTEX) ========
        Serial.println("\n=================== 🌿 GREEN FARM BKU 🌿 ===================");
        Serial.printf("[GLOBAL] Temp: %.1f C  |  Humid: %.1f%%  |  Water Lvl: %.1fcm\n",
                      loc_temp, loc_humid, loc_water);
        Serial.println("------------------------------------------------------");

        for (int i = 0; i < NUM_SECTION; i++)
        {
            // In thông số môi trường
            Serial.printf("[S%d] Soil: %d%% (DRY:%d-WET:%d) | Light: %d%% (TH:%d) | Temp TH: %.1f C\n",
                          i + 1,
                          loc_soil[i], loc_dry_th[i], loc_wet_th[i],
                          loc_light[i], loc_light_th[i],
                          loc_temp_th[i]);

            // In trạng thái thiết bị
            Serial.printf("     [DEV] Pump: %s (%s) | Fan: %s (%s) | LED: %s (%s, %d%%)\n",
                          loc_pump_on[i] ? "ON " : "OFF", loc_auto_pump[i] ? "AUTO" : "MANU",
                          loc_fan_on[i] ? "ON " : "OFF", loc_auto_fan[i] ? "AUTO" : "MANU",
                          loc_led_on[i] ? "ON " : "OFF", loc_auto_led[i] ? "AUTO" : "MANU", loc_led_br[i]);
        }
        Serial.println("======================================================\n");

        // ======== 3. DELAY 2.5 GIÂY TẠI ĐÂY ========
        vTaskDelay(pdMS_TO_TICKS(2500));
    }
}