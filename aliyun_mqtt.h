/*
  Aliyun_mqtt.h - Library for connect to Aliyun MQTT server with authentication by
  product key, device name and device secret.

  https://www.alibabacloud.com/help/product/30520.htm
*/

#ifndef _ALIYUN_MATT_H
#define _ALIYUN_MATT_H

#include <Arduino.h>
#include <PubSubClient.h>

/**
 * Connect to Alibaba Cloud MQTT server. In connection process, it will try several times for
 * possible network failure. For authentication issue, it will return false at once.
 *
 * @param mqttClient: Caller provide a valid PubSubClient object (initialized with network client).

 * @param productKey: Product Key, get from Alibaba Cloud Link Platform.

 * @param deviceName: Device Name, get from Alibaba Cloud Link Platform.

 * @param deviceSecret: Device Secret, get from Alibaba Cloud Link Platform.
 *
 * @param region: Optional region, use "cn-shanghai" as default. It can be "us-west-1",
 *                "ap-southeast-1" etc. Refer to Alibaba Cloud Link Platform.
 *
 *
 * @return true if connect succeed, otherwise false.
 */
extern "C" bool connectAliyunMQTT(
    PubSubClient &mqttClient,
    const char *productKey,
    const char *deviceName,
    const char *deviceSecret,
    const char *region = "cn-shanghai");

/**
 * Two new added APIs are designed for devices with limited resource like Arduino UNO.
 * Since it is hard to calculate HMAC256 on such devices, the calculation can be done externally.
 *
 * These two APIs should be used together with external HMAC256 calculation tools, e.g.
 * http://tool.oschina.net/encrypt?type=2
 * They can be used together to replace connectAliyunMQTT on resource-limited devices.
 */

/**
 * This API should be called in setup() phase to init all MQTT parameters. Since HMAC256
 * calculation is executed extenrally, a fixed timestamp string should be provided, such
 * as "23668" etc. The same timestamp string is also used to calculate HMAC256 result.
 *
 * Other params are similar to them in connectAliyunMQTT.
 */
extern "C" void mqttPrepare(
    const char *timestamp,
    const char *productKey,
    const char *deviceName,
    const char *deviceSecret,
    const char *region = "cn-shanghai");

/**
 * Use tools here to calculate HMAC256: http://tool.oschina.net/encrypt?type=2
 * The calculated result should be defined as constants and passed when call this function.
 */
extern "C" bool connectAliyunMQTTWithPassword(PubSubClient &mqttClient, const char *password);

#endif
