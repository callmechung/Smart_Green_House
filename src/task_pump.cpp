#include "task_pump.h"

void task_pump(void *pvParameter)
{
    
    int local_soil[NUM_SECTION];
    bool local_auto[NUM_SECTION];
    int wet_threshold[NUM_SECTION], dry_threshold[NUM_SECTION];
    bool local_pump_cmd[NUM_SECTION];
    

    for (int i = 0; i < NUM_SECTION; i++)
    {
        pinMode(section[i].pump_relay_pin, OUTPUT);
        digitalWrite(section[i].pump_relay_pin, LOW);
        
    }

    while (1)
    {

        // ======== Update local value =======
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            for (int i = 0; i < NUM_SECTION; i++)
            {
                local_soil[i] = section[i].soil_percent;
                local_auto[i] = section[i].is_auto_pump;
                wet_threshold[i] = section[i].soil_wet_threshold;
                dry_threshold[i] = section[i].soil_dry_threshold;
                local_pump_cmd[i] = section[i].is_pump_on; // Lệnh từ Web (nếu đang ở Manual)
            }
            xSemaphoreGive(xSensor);
        }

        // ======== Handle Logic ========
        for (int i = 0; i < NUM_SECTION; i++)
        {
            bool final_state = false;
            
            // ----- Auto Mode -----
            if (local_auto[i] == true)
            {
                if (local_soil[i] <= dry_threshold[i])
                    final_state = true;
                else if (local_soil[i] >= wet_threshold[i])
                    final_state = false;
                else 
                    final_state = local_pump_cmd[i];

                // Only update to server when final != local_pump_cmd
                if (final_state != local_pump_cmd[i])
                {
                    if (xSemaphoreTake(xSensor, portMAX_DELAY))
                    {
                        section[i].is_pump_on = final_state;
                        xSemaphoreGive(xSensor);
                    }
                }
            }
            // ----- Manual Mode -----
            else
            {
                final_state = local_pump_cmd[i];
            }

            // ----- Hardware action -----
            digitalWrite(section[i].pump_relay_pin, final_state ? HIGH : LOW);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}