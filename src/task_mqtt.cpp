#include "task_mqtt.h"

// Setup server
static const char *mqtt_server = "broker.hivemq.com";
static const uint16_t mqtt_port = 1883;

// Topic for PubSub
static const char *TOPIC_SENSOR = "greenframbku/sensors";
static const char *TOPIC_CMD = "greenframbku/cmd";

WiFiClient      espClient;
PubSubClient    client(espClient);

// ========== SNAPSHOT: luôn giữ bản sao mới nhất để publish ==========
// Được cập nhật mỗi khi sensor task ghi xong → publish NGAY lập tức
static float snap_temp = 0.0;
static float snap_humid = 0.0;
static float snap_water = 0.0;
static int snap_soil[NUM_SECTION] = {0};
static int snap_light[NUM_SECTION] = {0};
static int snap_pump[NUM_SECTION] = {false};
static int snap_led[NUM_SECTION] = {0};
static int snap_fan[NUM_SECTION] = {0};
void callback(char *topic, byte *payload, unsigned int length)
{
    // 1) Get message
    String message = "";
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.printf("[MQTT] CMD: %s\n", message.c_str());

    // 2) Do the task

    // ======= THRESHOLD COMMANDs ========
    // --- SECTION 1 ---
    if (message.startsWith("DRY_1_SET_"))
    {
        int v = constrain(message.substring(10).toInt(), 0, 100);
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[0].soil_dry_threshold = v;
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 1 - SET DRY THRESHOLD TO: %d%%\n", v);
    }
    else if (message.startsWith("WET_1_SET_"))
    {
        int v = constrain(message.substring(10).toInt(), section[0].soil_dry_threshold + 1, 100);
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[0].soil_wet_threshold = v;
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 1 - SET WET THRESHOLD TO: %d%%\n", v);
    }
    // --- SECTION 2 ---
    else if (message.startsWith("DRY_2_SET_"))
    {
        int v = constrain(message.substring(10).toInt(), 0, 100);
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[1].soil_dry_threshold = v;
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 2 - SET DRY THRESHOLD TO: %d%%\n", v);
    }
    else if (message.startsWith("WET_2_SET_"))
    {
        int v = constrain(message.substring(10).toInt(), section[1].soil_dry_threshold + 1, 100);
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[1].soil_wet_threshold = v;
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 2 - SET WET THRESHOLD TO: %d%%\n", v);
    }
    // --- SECTION 3 ---
    else if (message.startsWith("DRY_3_SET_"))
    {
        int v = constrain(message.substring(10).toInt(), 0, 100);
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[2].soil_dry_threshold = v;
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 3 - SET DRY THRESHOLD TO: %d%%\n", v);
    }
    else if (message.startsWith("WET_3_SET_"))
    {
        int v = constrain(message.substring(10).toInt(), section[2].soil_dry_threshold + 1, 100);
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[2].soil_wet_threshold = v;
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 3 - SET WET THRESHOLD TO: %d%%\n", v);
    }

    // --- LIGHT THRESHOLD ---
    else if (message.startsWith("LIGHT_1_TH_SET_"))
    {
        int v = constrain(message.substring(15).toInt(), 0, 100);
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[0].light_threshold = v;
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 1 - SET LIGHT THRESHOLD TO: %d%%\n", v);
    }
    else if (message.startsWith("LIGHT_2_TH_SET_"))
    {
        int v = constrain(message.substring(15).toInt(), 0, 100);
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[1].light_threshold = v;
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 2 - SET LIGHT THRESHOLD TO: %d%%\n", v);
    }
    else if (message.startsWith("LIGHT_3_TH_SET_"))
    {
        int v = constrain(message.substring(15).toInt(), 0, 100);
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[2].light_threshold = v;
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 3 - SET LIGHT THRESHOLD TO: %d%%\n", v);
    }

    // --- TEMP THRESHOLD  ---
    else if (message.startsWith("TEMP_1_TH_SET_"))
    {
        float v = message.substring(14).toFloat();
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[0].temp_threshold = v;
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 1 - SET TEMP THRESHOLD TO: %.1f°C\n", v);
    }
    else if (message.startsWith("TEMP_2_TH_SET_"))
    {
        float v = message.substring(14).toFloat();
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[1].temp_threshold = v;
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 2 - SET TEMP THRESHOLD TO: %.1f°C\n", v);
    }
    else if (message.startsWith("TEMP_3_TH_SET_"))
    {
        float v = message.substring(14).toFloat();
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[2].temp_threshold = v;
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 3 - SET TEMP THRESHOLD TO: %.1f°C\n", v);
    }

    // ======== PUMP COMMANDs ========
    // --- SECTION 1 ---
    else if (message == "PUMP_1_ON")
    {
        Serial.println("Section 1 - PUMP ON (Manual)");
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[0].is_auto_pump = false;
            section[0].is_pump_on = true;
            xSemaphoreGive(xSensor);
        }
    }
    else if (message == "PUMP_1_OFF")
    {
        Serial.println("Section 1 - PUMP OFF (Manual)");
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[0].is_auto_pump = false;
            section[0].is_pump_on = false;
            xSemaphoreGive(xSensor);
        }
    }
    else if (message == "PUMP_1_AUTO")
    {
        Serial.println("Section 1 - PUMP (Auto Mode)");
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[0].is_auto_pump = true;
            xSemaphoreGive(xSensor);
        }
    }
    // --- SECTION 2 ---
    else if (message == "PUMP_2_ON")
    {
        Serial.println("Section 2 - PUMP ON (Manual)");
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[1].is_auto_pump = false;
            section[1].is_pump_on = true;
            xSemaphoreGive(xSensor);
        }
    }
    else if (message == "PUMP_2_OFF")
    {
        Serial.println("Section 2 - PUMP OFF (Manual)");
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[1].is_auto_pump = false;
            section[1].is_pump_on = false;
            xSemaphoreGive(xSensor);
        }
    }
    else if (message == "PUMP_2_AUTO")
    {
        Serial.println("Section 2 - PUMP (Auto Mode)");
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[1].is_auto_pump = true;
            xSemaphoreGive(xSensor);
        }
    }
    // --- SECTION 3 ---
    else if (message == "PUMP_3_ON")
    {
        Serial.println("Section 3 - PUMP ON (Manual)");
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[2].is_auto_pump = false;
            section[2].is_pump_on = true;
            xSemaphoreGive(xSensor);
        }
    }
    else if (message == "PUMP_3_OFF")
    {
        Serial.println("Section 3 - PUMP OFF (Manual)");
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[2].is_auto_pump = false;
            section[2].is_pump_on = false;
            xSemaphoreGive(xSensor);
        }
    }
    else if (message == "PUMP_3_AUTO")
    {
        Serial.println("Section 3 - PUMP (Auto Mode)");
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[2].is_auto_pump = true;
            xSemaphoreGive(xSensor);
        }
    }

    // ======== LIGHT COMMANDs ========
    // --- SECTION 1 ---
    else if (message.startsWith("LIGHT_1_SET_"))
    {
        int percent = constrain(message.substring(12).toInt(), 0, 100);
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[0].is_auto_light = false;
            section[0].led_brightness = percent; // Lưu trữ bằng %
            section[0].is_light_on = (percent > 0);
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 1 - LED SET MANUAL TO %d%%\n", percent);
    }
    else if (message == "LIGHT_1_AUTO")
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[0].is_auto_light = true;
            xSemaphoreGive(xSensor);
        }
        Serial.println("Section 1 - LED AUTO MODE");
    }

    // --- SECTION 2 ---
    else if (message.startsWith("LIGHT_2_SET_"))
    {
        int percent = constrain(message.substring(12).toInt(), 0, 100);
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[1].is_auto_light = false;
            section[1].led_brightness = percent;
            section[1].is_light_on = (percent > 0);
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 2 - LED SET MANUAL TO %d%%\n", percent);
    }
    else if (message == "LIGHT_2_AUTO")
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[1].is_auto_light = true;
            xSemaphoreGive(xSensor);
        }
        Serial.println("Section 2 - LED AUTO MODE");
    }

    // --- SECTION 3 ---
    else if (message.startsWith("LIGHT_3_SET_"))
    {
        int percent = constrain(message.substring(12).toInt(), 0, 100);
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[2].is_auto_light = false;
            section[2].led_brightness = percent;
            section[2].is_light_on = (percent > 0);
            xSemaphoreGive(xSensor);
        }
        Serial.printf("Section 3 - LED SET MANUAL TO %d%%\n", percent);
    }
    else if (message == "LIGHT_3_AUTO")
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[2].is_auto_light = true;
            xSemaphoreGive(xSensor);
        }
        Serial.println("Section 3 - LED AUTO MODE");
    }

    // ======== FAN COMMANDs ========
    // --- SECTION 1 ---
    else if (message == "FAN_1_ON")
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[0].is_auto_fan = false;
            section[0].is_fan_on = true;
            xSemaphoreGive(xSensor);
        }
        Serial.println("Section 1 - FAN ON (Manual)");
    }
    else if (message == "FAN_1_OFF")
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[0].is_auto_fan = false;
            section[0].is_fan_on = false;
            xSemaphoreGive(xSensor);
        }
        Serial.println("Section 1 - FAN OFF (Manual)");
    }
    else if (message == "FAN_1_AUTO")
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[0].is_auto_fan = true;
            xSemaphoreGive(xSensor);
        }
        Serial.println("Section 1 - FAN AUTO MODE");
    }

    // --- SECTION 2 ---
    else if (message == "FAN_2_ON")
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[1].is_auto_fan = false;
            section[1].is_fan_on = true;
            xSemaphoreGive(xSensor);
        }
        Serial.println("Section 2 - FAN ON (Manual)");
    }
    else if (message == "FAN_2_OFF")
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[1].is_auto_fan = false;
            section[1].is_fan_on = false;
            xSemaphoreGive(xSensor);
        }
        Serial.println("Section 2 - FAN OFF (Manual)");
    }
    else if (message == "FAN_2_AUTO")
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[1].is_auto_fan = true;
            xSemaphoreGive(xSensor);
        }
        Serial.println("Section 2 - FAN AUTO MODE");
    }

    // --- SECTION 3 ---
    else if (message == "FAN_3_ON")
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[2].is_auto_fan = false;
            section[2].is_fan_on = true;
            xSemaphoreGive(xSensor);
        }
        Serial.println("Section 3 - FAN ON (Manual)");
    }
    else if (message == "FAN_3_OFF")
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[2].is_auto_fan = false;
            section[2].is_fan_on = false;
            xSemaphoreGive(xSensor);
        }
        Serial.println("Section 3 - FAN OFF (Manual)");
    }
    else if (message == "FAN_3_AUTO")
    {
        if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            section[2].is_auto_fan = true;
            xSemaphoreGive(xSensor);
        }
        Serial.println("Section 3 - FAN AUTO MODE");
    }
}

void connect_mqtt()
{
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    uint32_t t = millis();
    Serial.print("[MQTT] Connecting...");
    while (!client.connected() && millis() - t < 5000)
    {
        String clientID = "ESP32_Garden_" + String(random(0xffff), HEX);

        if (client.connect(clientID.c_str()))
        {
            Serial.println("Succesfull");
            isMqttConnected = true;

            // Subcrising after connected
            client.subscribe(TOPIC_CMD);
            return;
        }
        else
        {
            Serial.print(".");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    if (!client.connected())
    {
        Serial.println("[MQTT] Connect timeout - retry later !!!");
        isMqttConnected = false;
    }
}

static void publish_snapshot()
{
    JsonDocument doc;

    JsonObject air = doc["air"].to<JsonObject>();
    air["t"] = snap_temp;
    air["h"] = snap_humid;

    doc["water_lvl"] = snap_water;

    for (int i = 0; i < NUM_SECTION; i++)
    {
        String sec_name = "s" + String(i + 1);
        JsonObject sec_obj = doc[sec_name].to<JsonObject>();
        sec_obj["soil"] = snap_soil[i];
        sec_obj["light"] = snap_light[i];
        sec_obj["pump"] = snap_pump[i];
        sec_obj["led"] = snap_led[i];
        sec_obj["fan"] = snap_fan[i];
    }

    String payload;
    serializeJson(doc, payload);

    if (client.publish(TOPIC_SENSOR, payload.c_str()))
    {
        Serial.print("[MQTT] Published: ");
        Serial.println(payload);
    }
    else
    {
        Serial.println("[MQTT] ❌ Publish FAILED");
    }
}

void task_mqtt(void *pvParameter)
{
    // ======== Set up server and Initialize variables ========

    Serial.println("[MQTT] Waiting for WiFi...");
    while (!isWifiConnected)
        vTaskDelay(pdMS_TO_TICKS(500));

    connect_mqtt();

    static uint32_t last_publish_ms = 0;
    const uint32_t PUBLISH_INTERVAL_MS = 10000; // publish each 10s

    while (1)
    {
        // ======== Step 1: Checking connection ========
        if (!isWifiConnected)
        {
            isMqttConnected = false;
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        if (!isMqttConnected || !client.connected())
        {
            connect_mqtt();
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        
        // ======== Step 2: Process incoming commands ========
        client.loop();

        // ======== Step 3: Publish data ========
        uint32_t now = millis();
        if (now - last_publish_ms >= PUBLISH_INTERVAL_MS)
        {
            // --- Take value ---
            if (xSensor != NULL &&
                xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
            {
                snap_temp = air_temp;
                snap_humid = air_humid;
                snap_water = water_level;

                for (int i = 0; i < NUM_SECTION; i++)
                {
                    snap_soil[i] = section[i].soil_percent;
                    snap_light[i] = section[i].light_percent;
                    snap_pump[i] = section[i].is_pump_on ? 1 : 0;
                    snap_led[i] = section[i].led_brightness;
                    snap_fan[i] = section[i].is_fan_on ? 1 : 0;
                }

                xSemaphoreGive(xSensor);
           }

           publish_snapshot();
           last_publish_ms = now;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}