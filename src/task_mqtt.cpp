#include "task_mqtt.h"

// Setup server
static const char *mqtt_server = "broker.hivemq.com";
static const uint16_t mqtt_port = 1883;

static const uint32_t PUBLISH_INTERVAL_MS = 5000; 

// Topic for PubSub
static const char *TOPIC_SENSOR = "greenframbku/sensors";
static const char *TOPIC_CMD = "greenframbku/cmd";

WiFiClient espClient;
PubSubClient client(espClient);

// ========== SNAPSHOT: luôn giữ bản sao mới nhất để publish ==========
// Được cập nhật mỗi khi sensor task ghi xong → publish NGAY lập tức
static float snap_temp = 0.0;
static float snap_humid = 0.0;
static float snap_water = 0.0;
static int snap_soil[NUM_SECTION] = {0};
static int snap_light[NUM_SECTION] = {0};
static int snap_pump_status[NUM_SECTION] = {0};         // is_pump_on
static int snap_fan_status[NUM_SECTION] = {0};          // is_fan_on
static int snap_led_status[NUM_SECTION] = {0};          // is_led_on
static int snap_led_brightness[NUM_SECTION] = {0};      // led_brightness

// Biến toàn cục đếm thời gian publish
static uint32_t last_publish_ms = 0;

void callback(char *topic, byte *payload, unsigned int length)
{
    // 1) Get message
    String message = "";
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.printf("[MQTT] CMD: %s\n", message.c_str());

    // 2) Parse and Execute the Command
    if (xSensor != NULL && xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
    {
        // ===== PUMP COMMAND ====
        // --- section1 ---
        if (message == "PUMP_1_ON") {section[0].is_pump_on = true; section[0].is_auto_pump = false;}
        else if (message == "PUMP_1_OFF") {section[0].is_pump_on = false; section[0].is_auto_pump = false;}
        else if (message == "PUMP_1_AUTO") {section[0].is_pump_on = false; section[0].is_auto_pump = true;}
        // --- section2 ---
        else if (message == "PUMP_2_ON") {section[1].is_pump_on = true; section[1].is_auto_pump = false;}
        else if (message == "PUMP_2_OFF") {section[1].is_pump_on = false; section[1].is_auto_pump = false;}
        else if (message == "PUMP_2_AUTO") {section[1].is_pump_on = false; section[1].is_auto_pump = true;}
        // --- section3 ---
        else if (message == "PUMP_3_ON") {section[2].is_pump_on = true; section[2].is_auto_pump = false;}
        else if (message == "PUMP_3_OFF") {section[2].is_pump_on = false; section[2].is_auto_pump = false;}
        else if (message == "PUMP_3_AUTO") {section[2].is_pump_on = false; section[2].is_auto_pump = true;}

        // ===== FAN COMMAND =====
        // --- section1 ---
        else if (message == "FAN_1_ON") {section[0].is_fan_on = true; section[0].is_auto_fan = false;}
        else if (message == "FAN_1_OFF") {section[0].is_fan_on = false; section[0].is_auto_fan = false;}
        else if (message == "FAN_1_AUTO") {section[0].is_fan_on = false; section[0].is_auto_fan = true;}
        // --- section2 ---
        else if (message == "FAN_2_ON") {section[1].is_fan_on = true; section[1].is_auto_fan = false;}
        else if (message == "FAN_2_OFF") {section[1].is_fan_on = false; section[1].is_auto_fan = false;}
        else if (message == "FAN_2_AUTO") {section[1].is_fan_on = false; section[1].is_auto_fan = true;}
        // --- section3 ---
        else if (message == "FAN_3_ON") {section[2].is_fan_on = true; section[2].is_auto_fan = false;}
        else if (message == "FAN_3_OFF") {section[2].is_fan_on = false; section[2].is_auto_fan = false;}
        else if (message == "FAN_3_AUTO") {section[2].is_fan_on = false; section[2].is_auto_fan = true;}

        // ===== LED COMMAND ====

        // --- section1 ---
        else if (message.startsWith("LED_1_SET_"))
        {
            int b = constrain(message.substring(10).toInt(), 0, 100);
            section[0].led_brightness = b;
            section[0].is_auto_led = false;
            section[0].is_led_on = (b > 0);
        }
        else if (message == "LED_1_AUTO") {section[0].is_auto_led = true;}

        // --- section2 ---
        else if (message.startsWith("LED_2_SET_"))
        {
            int b = constrain(message.substring(10).toInt(), 0, 100);
            section[1].led_brightness = b;
            section[1].is_auto_led = false;
            section[1].is_led_on = (b > 0);
        }
        else if (message == "LED_2_AUTO") {section[1].is_auto_led = true;}

        // --- section3 ---
        else if (message.startsWith("LED_3_SET_"))
        {
            int b = constrain(message.substring(10).toInt(), 0, 100);
            section[2].led_brightness = b;
            section[2].is_auto_led = false;
            section[2].is_led_on = (b > 0);
        }
        else if (message == "LED_3_AUTO") {section[2].is_auto_led = true;}


        // ===== THRESHOLD =====
        
        // --- section1 ---
        else if (message.startsWith("DRY_1_SET_")) {section[0].soil_dry_threshold = constrain(message.substring(10).toInt(), 0, 100);}
        else if (message.startsWith("WET_1_SET_")) {section[0].soil_wet_threshold = constrain(message.substring(10).toInt(), 0, 100);}
        else if (message.startsWith("LIGHT_1_SET_")) {section[0].light_threshold = constrain(message.substring(12).toInt(), 0, 100);}
        else if (message.startsWith("TEMP_1_SET_")) {section[0].temp_threshold = message.substring(11).toFloat();}

        // --- section2 ---
        else if (message.startsWith("DRY_2_SET_")) {section[1].soil_dry_threshold = constrain(message.substring(10).toInt(), 0, 100);}
        else if (message.startsWith("WET_2_SET_")) {section[1].soil_wet_threshold = constrain(message.substring(10).toInt(), 0, 100);}
        else if (message.startsWith("LIGHT_2_SET_")) {section[1].light_threshold = constrain(message.substring(12).toInt(), 0, 100);}
        else if (message.startsWith("TEMP_2_SET_")) {section[1].temp_threshold = message.substring(11).toFloat();}

        // --- section3 ---
        else if (message.startsWith("DRY_3_SET_")) {section[2].soil_dry_threshold = constrain(message.substring(10).toInt(), 0, 100);}
        else if (message.startsWith("WET_3_SET_")) {section[2].soil_wet_threshold = constrain(message.substring(10).toInt(), 0, 100);}
        else if (message.startsWith("LIGHT_3_SET_")) {section[2].light_threshold = constrain(message.substring(12).toInt(), 0, 100);}
        else if (message.startsWith("TEMP_3_SET_")) {section[2].temp_threshold = message.substring(11).toFloat();}

        xSemaphoreGive(xSensor);
    }

    // Ép MQTT bắn dữ liệu ngay lập tức sau khi nhận lệnh
    last_publish_ms = 0;
}

void connect_mqtt()
{
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    client.setBufferSize(1024);

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
        sec_obj["pump_status"] = snap_pump_status[i];
        sec_obj["led_status"] = snap_led_status[i];
        sec_obj["fan_status"] = snap_fan_status[i];
        sec_obj["led_brightness"] = snap_led_brightness[i];
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
                    snap_pump_status[i] = section[i].is_pump_on ? 1 : 0;
                    snap_fan_status[i] = section[i].is_fan_on ? 1 : 0;
                    snap_led_status[i] = section[i].is_led_on ? 1 : 0;
                    snap_led_brightness[i] = section[i].led_brightness;
                }

                xSemaphoreGive(xSensor);
            }

            publish_snapshot();
            last_publish_ms = now;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}