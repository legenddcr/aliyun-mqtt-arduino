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

bool connectAliyunMQTT(
  PubSubClient &mqttClient,
  const char *productKey,
  const char *deviceName,
  const char *deviceSecret,
  const char *region)
{
  // Init MQTT broker
  String mqttBroker = productKey;
  mqttBroker += ".iot-as-mqtt.";
  mqttBroker += String(region);
  mqttBroker += ".aliyuncs.com";

  // Generate MQTT Username, it can be device SN etc.
  String mqttUserName = deviceName;
  mqttUserName += '&';
  mqttUserName += productKey;

  // Generate MQTT client ID
  String timestamp = String(millis());

  String clientID = deviceName; // Device SN etc. can be used
  String mqttClientID = clientID;
  mqttClientID += "|securemode=3,signmethod=hmacsha256,timestamp=";
  mqttClientID += timestamp;
  mqttClientID += '|';

  // Generate MQTT Password
  String signcontent = "clientId";
  signcontent += clientID;
  signcontent += "deviceName";
  signcontent += deviceName;
  signcontent += "productKey";
  signcontent += productKey;
  signcontent += "timestamp";
  signcontent += timestamp;

  // Serial.print("HMAC256 data: ");
  // Serial.println(signcontent);
  // Serial.print("HMAC256 key: ");
  // Serial.println(deviceSecret);
  String mqttPassword = hmac256(signcontent, deviceSecret);

  // Serial.println("=================");
  // Serial.println(mqttBroker);
  // Serial.println(MQTT_PORT);
  // Serial.print("MQTT Client ID: ");
  // Serial.println(mqttClientID);
  // Serial.print("MQTT User Name: ");
  // Serial.println(mqttUserName);
  // Serial.print("MQTT Password: ");
  // Serial.println(mqttPassword);

  mqttClient.setServer(mqttBroker.c_str(), MQTT_PORT);

  byte mqttConnectTryCnt = 5;
  while (!mqttClient.connected() && mqttConnectTryCnt > 0)
  {
    Serial.println("Connecting to MQTT Server ...");
    if (mqttClient.connect(mqttClientID.c_str(), mqttUserName.c_str(), mqttPassword.c_str()))
    {

      Serial.println("MQTT Connected!");
      return true;
    }
    else
    {
      byte errCode = mqttClient.state();
      Serial.print("MQTT connect failed, error code:");
      Serial.println(errCode);
      if (errCode == MQTT_CONNECT_BAD_PROTOCOL
        || errCode == MQTT_CONNECT_BAD_CLIENT_ID
        || errCode == MQTT_CONNECT_BAD_CREDENTIALS
        || errCode == MQTT_CONNECT_UNAUTHORIZED)
        break; // No need to try again for these situation
      delay(5000);
    }
    mqttConnectTryCnt -= 1;
  }

  return false;
}
