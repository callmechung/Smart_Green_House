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
static bool snap_pump[NUM_SECTION] = {false};
static int snap_led[NUM_SECTION] = {0};

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

    // ======== PUMP COMMANDs ========
    if (message == "PUMP_1_ON")
    {
        Serial.println("Section 1 - PUMP ON");
        section[0].is_pump_on = true;
        digitalWrite(section[0].pump_relay_pin, HIGH);
    }
    else if (message == "PUMP_2_ON")
    {
        Serial.println("Section 2 - PUMP ON");
        section[1].is_pump_on = true;
        digitalWrite(section[1].pump_relay_pin, HIGH);
    }
    else if (message == "PUMP_3_ON")
    {
        Serial.println("Section 3 - PUMP ON");
        section[2].is_pump_on = true;
        digitalWrite(section[2].pump_relay_pin, HIGH);
    }
    else if (message == "PUMP_1_OFF")
    {
        Serial.println("Section 1 - PUMP OFF");
        section[0].is_pump_on = false;
        digitalWrite(section[0].pump_relay_pin, LOW);
    }
    else if (message == "PUMP_2_OFF")
    {
        Serial.println("Section 2 - PUMP OFF");
        section[1].is_pump_on = false;
        digitalWrite(section[1].pump_relay_pin, LOW);
    }
    else if (message == "PUMP_3_OFF")
    {
        Serial.println("Section 3 - PUMP OFF");
        section[2].is_pump_on = false;
        digitalWrite(section[2].pump_relay_pin, LOW);
    }

    // ======== LIGHT COMMANDs ========

    else if (message.startsWith("LIGHT_1_SET_"))
    {
        int v = constrain(message.substring(12).toInt(), 0, 255);
        section[0].led_brightness = v;
        section[0].is_light_on = (v != 0);
        analogWrite(section[0].light_ctrl_pin, v);
        Serial.printf("Section 1 - LED SET %d\n", v);
    }
    else if (message.startsWith("LIGHT_2_SET_"))
    {
        int v = constrain(message.substring(12).toInt(), 0, 255);
        section[1].led_brightness = v;
        section[1].is_light_on = (v != 0);
        analogWrite(section[1].light_ctrl_pin, v);
        Serial.printf("Section 2 - LED SET %d\n", v);
    }
    else if (message.startsWith("LIGHT_3_SET_"))
    {
        int v = constrain(message.substring(12).toInt(), 0, 255);
        section[2].led_brightness = v;
        section[2].is_light_on = (v != 0);
        analogWrite(section[2].light_ctrl_pin, v);
        Serial.printf("Section 3 - LED SET %d\n", v);
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
                    snap_pump[i] = section[i].is_pump_on;
                    snap_led[i] = section[i].led_brightness;
                }

                xSemaphoreGive(xSensor);
           }

           publish_snapshot();
           last_publish_ms = now;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}