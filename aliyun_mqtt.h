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
#endif
