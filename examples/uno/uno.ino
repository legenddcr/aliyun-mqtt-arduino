/*
 * Arduino Temperature with DHT sensors to Aliyun MQTT Server
 * Refer to https://thingsboard.io/docs/samples/arduino/temperature/
 */
#include <Arduino.h>
#include <DHT.h>

#include <SoftwareSerial.h>
#include <WiFiEspClient.h>
#include <WiFiEsp.h>

#include <aliyun_mqtt.h>

//DHT sensor pin and type setting
#define DHTPIN A0
#define DHTTYPE DHT11

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

#define WIFI_SSID "<YOUR WIFI SSID>"
#define WIFI_PASSWORD "<YOUR WIFI PASSWORD>"

// Initialize the Ethernet client object
const int WIFI_RX = 2;
const int WIFI_TX = 3;
SoftwareSerial softSerial(WIFI_RX, WIFI_TX); // RX, TX

void connectWiFi()
{
    int status = WiFi.status();
    // attempt to connect to WiFi network
    while (status != WL_CONNECTED)
    {
        Serial.print(F("Attempting to connect to Wifi: "));
        Serial.println(WIFI_SSID);
        // Connect to WPA/WPA2 network
        status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        delay(500);
    }
}

void initWiFi()
{
    // initialize serial for ESP module
    softSerial.begin(9600);
    // initialize ESP module
    WiFi.init(&softSerial);
    // check for the presence of the shield
    if (WiFi.status() == WL_NO_SHIELD)
    {
        Serial.println(F("WiFi shield not present, stop"));
        // don't continue
        while (true);
    }

    connectWiFi();
    Serial.print(F("Connected to AP: "));
    Serial.println(WiFi.localIP());
}

#define TIMESTAMP "23668" // can be changed to a random positive value string

// Use external HMAC256 calculation for MQTT_PASSWORD: http://tool.oschina.net/encrypt?type=2
// Content: clientId[DEVICE_NAME]deviceName[DEVICE_NAME]productKey[PRODUCT_KEY]timestamp[TIMESTAMP]
// Key: [DEVICE_SECRET]

#define PRODUCT_KEY "<YOUR PRODUCT KEY>"
#define DEVICE_NAME "<YOUR DEVICE NAME>"
#define DEVICE_SECRET "<YOUR DEVICE SECRET>"
#define MQTT_PASSWORD "<YOUR EXTERNAL CALCULAED PASSWORD>"

// https://help.aliyun.com/document_detail/89301.html
#define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"

unsigned long lastSend;

WiFiEspClient espClient;
PubSubClient mqttClient(espClient);

void checkMqttConnection()
{
    connectWiFi();

    // Loop until we're reconnected
    while (!mqttClient.connected())
    {
        Serial.println(F("Connecting to Aliyun IoT ..."));
        // Attempt to connect (clientId, username, password)
        if (connectAliyunMQTTWithPassword(mqttClient, MQTT_PASSWORD))
        {
            Serial.println("[MQTT Connected]");
        }
        else
        {
            Serial.print(F("[MQTT Connect FAILED] [rc = "));
            Serial.print(mqttClient.state());
            Serial.println(F(": retrying in 5 seconds]"));
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void getAndSendTemperatureAndHumidityData()
{
    // Reading temperature or humidity takes about 250 milliseconds!
    int h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    int t = dht.readTemperature();
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t))
    {
        Serial.println(F("Failed to read sensor data!"));
        return;
    }

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.println(F("°C"));

    String temperature = String(t);
    String humidity = String(h);

    // Prepare a JSON payload string
    String payload = "{";
    payload += "\"temperature\":";
    payload += temperature;
    payload += ",";
    payload += "\"humidity\":";
    payload += humidity;
    payload += "}";

    // Send payload as Alink data
    String jsonData = F("{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":");
    jsonData += payload;
    jsonData += F("}");

    char alinkData[128];
    jsonData.toCharArray(alinkData, 128);
    mqttClient.publish(ALINK_TOPIC_PROP_POST, alinkData);
    Serial.println(alinkData);
}

void setup()
{
    Serial.begin(115200);
    initWiFi();
    dht.begin();
    mqttPrepare(TIMESTAMP, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET);
    lastSend = 0;
}

void loop()
{
    checkMqttConnection();

    if (millis() - lastSend >= 10000)
    { // Update and send only after 1 seconds
        getAndSendTemperatureAndHumidityData();
        lastSend = millis();
    }
    mqttClient.loop();
}
