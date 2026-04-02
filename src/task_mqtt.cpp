#include "task_mqtt.h"

// Setup server
static const char *mqtt_server = "broker.hivemq.com";
static const uint16_t mqtt_port = 1883;

// Topic for PubSub
static const char *TOPIC_SENSOR = "greenframbku/sensors";
static const char *TOPIC_CMD = "greenframbku/cmd";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("New message: ");
    Serial.println(topic);

    // 1) Get message
    String message = "";
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.print("The message is: ");
    Serial.println(message);

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
        String string_light_val = message.substring(12);
        int light_val = string_light_val.toInt();

        if (light_val < 0)
            light_val = 0;
        if (light_val > 255)
            light_val = 255;

        Serial.printf("Section 1 - SET LED TO %d\n", light_val);

        section[0].led_brightness = light_val;
        section[0].is_light_on = (light_val != 0);

        analogWrite(section[0].light_ctrl_pin, section[0].led_brightness);
    }
    else if (message.startsWith("LIGHT_2_SET_"))
    {
        String string_light_val = message.substring(12);
        int light_val = string_light_val.toInt();

        if (light_val < 0)
            light_val = 0;
        if (light_val > 255)
            light_val = 255;

        Serial.printf("Section 2 - SET LED TO %d\n", light_val);

        section[1].led_brightness = light_val;
        section[1].is_light_on = (light_val != 0);

        analogWrite(section[1].light_ctrl_pin, section[1].led_brightness);
    }
    else if (message.startsWith("LIGHT_3_SET_"))
    {
        String string_light_val = message.substring(12);
        int light_val = string_light_val.toInt();

        if (light_val < 0)
            light_val = 0;
        if (light_val > 255)
            light_val = 255;

        Serial.printf("Section 3 - SET LED TO %d\n", light_val);

        section[2].led_brightness = light_val;
        section[2].is_light_on = (light_val != 0);

        analogWrite(section[2].light_ctrl_pin, section[2].led_brightness);
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
        }
        else
        {
            Serial.print(".");
            Serial.println(client.state());
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    if (!client.connected())
    {
        Serial.println("[MQTT] Connect timeout, retry later !!!");
        isMqttConnected = false;
    }
}

void task_mqtt(void *pvParameter)
{
    // ======== Set up server and Initialize variables ========

    connect_mqtt();

    float cur_temp, cur_humid, cur_water_level;
    int cur_soil_humid[NUM_SECTION], cur_light[NUM_SECTION], cur_led[NUM_SECTION];
    bool cur_pump[NUM_SECTION];

    while (1)
    {
        // ======== Step 1: Checking connection ========
        if (!isWifiConnected || !isMqttConnected)
        {
            if (isWifiConnected)
            {
                connect_mqtt();
            }
            vTaskDelay(pdMS_TO_TICKS(1000)); //Wait for stable
            continue;
        }

        client.loop();

        // ======== Step 2: Update value ========

        if (isMqttConnected && xSensor != NULL &&
            xSemaphoreTake(xSensor, portMAX_DELAY) == pdPASS)
        {
            cur_temp = air_temp;
            cur_humid = air_humid;
            cur_water_level = water_level;

            for (int i = 0; i < NUM_SECTION; i++)
            {
                cur_soil_humid[i] = section[i].soil_percent;
                cur_light[i] = section[i].light_percent;
                cur_pump[i] = section[i].is_pump_on;
                cur_led[i] = section[i].led_brightness;
            }

            xSemaphoreGive(xSensor);
        }

        // ======== Step 3: Format Payload ========
        static uint32_t last_publish_time = 0;
        // Publish 10s 1 lan
        if (isMqttConnected && (millis() - last_publish_time) > 10000)
        {
            // Initialized a JSON document
            JsonDocument doc;

            // Preapare for payload
            JsonObject air = doc["air"].to<JsonObject>();
            air["t"] = cur_temp;
            air["h"] = cur_humid;

            doc["water_lvl"] = cur_water_level;

            for (int i = 0; i < NUM_SECTION; i++)
            {
                // Section name: "s1", "s2", "s3"
                String sec_name = "s" + String(i + 1);

                // Tạo một Object con cho từng section
                JsonObject sec_obj = doc[sec_name].to<JsonObject>();

                // Nhét data vào Object con đó
                sec_obj["soil"] = cur_soil_humid[i];
                sec_obj["light"] = cur_light[i];
                sec_obj["pump"] = cur_pump[i];
                sec_obj["led"] = cur_led[i];
            }

            // Make Payload
            String payload;
            serializeJson(doc, payload);

            // Publish to Broker
            Serial.print("Publishing Payload: ");
            Serial.println(payload);

            client.publish(TOPIC_SENSOR, payload.c_str());

            last_publish_time = millis();
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}