#include "task_WIFI.h"

static const char *WIFI_SSID = "CALLMECHUNG";
static const char *WIFI_PASS = "28032005";

void connect_wifi()
{
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    uint32_t t = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t < 5000)
    {
        Serial.print(".");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.printf("WiFi connect time out after %d second. Retrying later.\n", 5);
        isWifiConnected = false;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi connected successfully");
        isWifiConnected = true;
    }
}

void task_wifi(void *pvParamter)
{
    while (1)
    {
        if (WiFi.status() != WL_CONNECTED || !isWifiConnected)
        {
            isWifiConnected = false;
            Serial.print("WiFi is connecting");
            connect_wifi();
        }
        else
        {
            Serial.println("WiFi still on");
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}