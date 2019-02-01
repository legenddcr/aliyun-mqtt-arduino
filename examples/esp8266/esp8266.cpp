#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "aliyun_mqtt.h"

// GPIO 13, D7 on the Node MCU v3
#define SENSOR_PIN 13

#define WIFI_SSID "<REPLACE WITH YOUR NEWWORK SSID>"
#define WIFI_PASSWD "<REPLACE WITH YOUR NEWWORK PASSWORD>"

#define PRODUCT_KEY "<REPLACE WITH YOUR PRODUCT KEY>"
#define DEVICE_NAME "<REPLACE WITH YOUR DEVICE NAME>"
#define DEVICE_SECRET "<REPLACE WITH YOUR DEVICE SECRET>"

#define ALINK_BODY_FORMAT "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"%s\",\"params\":%s}"
#define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"
#define ALINK_TOPIC_PROP_POSTRSP "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post_reply"
#define ALINK_TOPIC_PROP_SET "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/service/property/set"
#define ALINK_METHOD_PROP_POST "thing.event.property.post"

unsigned long lastMs = 0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void initWifi(const char *ssid, const char *password)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi does not connect, try again ...");
        delay(3000);
    }

    Serial.println("Wifi is connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    payload[length] = '\0';
    Serial.println((char *)payload);

    if (strstr(topic, ALINK_TOPIC_PROP_SET))
    {
        StaticJsonBuffer<100> jsonBuffer;
        JsonObject &root = jsonBuffer.parseObject(payload);
        if (!root.success())
        {
            Serial.println("parseObject() failed");
            return;
        }
    }
}

void mqttCheckConnect()
{
    while (!connectAliyunMQTT(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET))
    {
    }

    Serial.println("MQTT connect succeed!");
    // client.subscribe(ALINK_TOPIC_PROP_POSTRSP);
    mqttClient.subscribe(ALINK_TOPIC_PROP_SET);
    Serial.println("subscribe done");
}

void mqttIntervalPost()
{
    char param[32];
    char jsonBuf[128];

    sprintf(param, "{\"MotionAlarmState\":%d}", digitalRead(13));
    sprintf(jsonBuf, ALINK_BODY_FORMAT, ALINK_METHOD_PROP_POST, param);
    Serial.println(jsonBuf);
    mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
}

void setup()
{

    pinMode(SENSOR_PIN, INPUT);
    /* initialize serial for debugging */
    Serial.begin(115200);

    Serial.println("Demo Start");

    initWifi(WIFI_SSID, WIFI_PASSWD);

    mqttClient.setCallback(callback);
}

// the loop function runs over and over again forever
void loop()
{
    if (millis() - lastMs >= 5000)
    {
        lastMs = millis();
        mqttCheckConnect();

        /* Post */
        mqttIntervalPost();
    }

    mqttClient.loop();

    unsigned int WAIT_MS = 60000;
    if (digitalRead(SENSOR_PIN) == HIGH)
    {
        Serial.println("Motion detected!");
    }
    else
    {
        Serial.println("Motion absent!");
    }
    delay(WAIT_MS); // ms
    Serial.println(millis() / WAIT_MS);
}
