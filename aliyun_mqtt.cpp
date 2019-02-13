/*
  Aliyun_mqtt.h - Library for connect to Aliyun MQTT server.
*/

#include "aliyun_mqtt.h"

#include <SHA256.h>

#define MQTT_PORT 1883
#define SHA256HMAC_SIZE 32

// Verify tool: http://tool.oschina.net/encrypt?type=2
static String hmac256(const String &signcontent, const String &ds)
{
  byte hashCode[SHA256HMAC_SIZE];
  SHA256 sha256;

  const char *key = ds.c_str();
  size_t keySize = ds.length();

  sha256.resetHMAC(key, keySize);
  sha256.update((const byte *)signcontent.c_str(), signcontent.length());
  sha256.finalizeHMAC(key, keySize, hashCode, sizeof(hashCode));

  String sign = "";
  for (byte i = 0; i < SHA256HMAC_SIZE; ++i)
  {
    sign += "0123456789ABCDEF"[hashCode[i] >> 4];
    sign += "0123456789ABCDEF"[hashCode[i] & 0xf];
  }

  return sign;
}

static String mqttBroker;
static String mqttClientID;
static String mqttUserName;
static String mqttPassword;

// call this function once
void mqttPrepare(const char *timestamp,
                 const char *productKey,
                 const char *deviceName,
                 const char *deviceSecret,
                 const char *region)
{
  mqttBroker = productKey;
  mqttBroker += ".iot-as-mqtt.";
  mqttBroker += String(region);
  mqttBroker += ".aliyuncs.com";
  // Serial.println(mqttBroker);

  mqttUserName = deviceName;
  mqttUserName += '&';
  mqttUserName += productKey;
  // Serial.println(mqttUserName);

  mqttClientID = deviceName; // device name used as client ID
  mqttClientID += "|securemode=3,signmethod=hmacsha256,timestamp=";
  mqttClientID += timestamp;
  mqttClientID += '|';
  // Serial.println(mqttClientID);
}

bool connectAliyunMQTTWithPassword(PubSubClient &mqttClient, const char *password)
{
  mqttClient.setServer(mqttBroker.c_str(), MQTT_PORT);

  byte mqttConnectTryCnt = 5;
  while (!mqttClient.connected() && mqttConnectTryCnt > 0)
  {
    Serial.println("Connecting to MQTT Server ...");
    if (mqttClient.connect(mqttClientID.c_str(), mqttUserName.c_str(), password))
    {

      // Serial.println("MQTT Connected!");
      return true;
    }
    else
    {
      byte errCode = mqttClient.state();
      Serial.print("MQTT connect failed, error code:");
      Serial.println(errCode);
      if (errCode == MQTT_CONNECT_BAD_PROTOCOL || errCode == MQTT_CONNECT_BAD_CLIENT_ID || errCode == MQTT_CONNECT_BAD_CREDENTIALS || errCode == MQTT_CONNECT_UNAUTHORIZED)
      {
        Serial.println("No need to try again.");
        break; // No need to try again for these situation
      }
      delay(5000);
    }
    mqttConnectTryCnt -= 1;
  }

  return false;
}

bool connectAliyunMQTT(
    PubSubClient &mqttClient,
    const char *productKey,
    const char *deviceName,
    const char *deviceSecret,
    const char *region)
{
  String timestamp = String(millis());
  mqttPrepare(timestamp.c_str(), productKey, deviceName, deviceSecret, region);

  // Generate MQTT Password, use deviceName as clientID
  String signcontent = "clientId";
  signcontent += deviceName;
  signcontent += "deviceName";
  signcontent += deviceName;
  signcontent += "productKey";
  signcontent += productKey;
  signcontent += "timestamp";
  signcontent += timestamp;

  String mqttPassword = hmac256(signcontent, deviceSecret);

  // Serial.print("HMAC256 data: ");
  // Serial.println(signcontent);
  // Serial.print("HMAC256 key: ");
  // Serial.println(deviceSecret);
  // Serial.println(mqttPassword);

  return connectAliyunMQTTWithPassword(mqttClient, mqttPassword.c_str());
}
