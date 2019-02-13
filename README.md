# Arduino - 阿里云物联网平台MQTT连接库

## 简介

为简化使用Arduino Framwork的开发者连接阿里云物联网平台时的工作，本项目封装了开源的
[PubSubClient库](https://github.com/knolleary/pubsubclient)和
[Arduino Cryptography Library](https://github.com/rweather/arduinolibs)代码库，
帮助开发者完成连接阿里云物联网平台需要的参数设置以及准备工作。开发者只要提供从阿里云物联网平台上获得的
三元组（Product Key, Device Name, Device Key）就可以迅速使用支持Arduino Framework的开发板完
成MQTT连云和数据上报。

关于本库实现的说明：

1. 本代码库使用HMAC256算法完成连接时的签名验证。
2. 本代码库并不进一步封装`PubSubClient`库本身的接口，使用者可以仍然以熟悉的方式使用`PubSubClient`库接口，
   这个库仅仅帮助开发者完成连接阿里云MQTT Broker的繁琐部分。
3. 默认使用阿里云华东2区作为连接region，使用其他region的用户调用时可以显式指定region名。

目前已验证平台和硬件包括:

* Atmel AVR: Arduino UNO.
* Espressif 8266: Node MCU v3.

其他经验证的平台和开发板信息可以逐步更新到上面列表中。

## 使用方法

### 资源较多的设备

在Node MCU上可以直接使用本库提供的API完成所有连接参数以及HMAC256签名的动态计算，仅仅使用下面的API即可。

```C
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
bool connectAliyunMQTT(
    PubSubClient &mqttClient,
    const char *productKey,
    const char *deviceName,
    const char *deviceSecret,
    const char *region = "cn-shanghai");
```

### 资源较少的设备

如Arduino UNO，只有2K的SRAM，对于此类设备，需要将计算HMAC256签名的过程放在程序外部，并对应用程序内存使用做适当的优化，才能运行程序。

资源受限平台上，本库提供了以下两个APIs，其中初始化准备阶段如Arduino `setup`函数中调用第一个函数准备MQTT的连接参数。另外借助[外部签名
计算工具](http://tool.oschina.net/encrypt?type=2）完成HMAC256 MQTT签名的计算，之后使用该签名进行MQTT连接过程。

```C
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
```

### 包含头文件

使用者只需要将`aliyun_mqtt.h`包含到自己的文件中使用即可，无需进一步包含`PubSubClient.h` 头文件。
已支持开发板中的使用示例可参考example目录中对应的例子。

### 增加库依赖和编译选项配置

由于Aliyun IoT平台MQTT borker连接时需要的参数与`PubSubClient`库默认参数不同，因此需要将以下两个宏定义加入你对应工程的预编译选项中:

* KeepAlive: 应当在60s到300秒之间，否则会被服务器拒绝连接。
* 默认MQTT包最大值：PubSubClient库默认为128字节，需要调整为256字节及以上值。

例如对于Platform IO的工程，可以在应用的`platform.ini`文件中加入类似下面的库依赖和编译选项配置：

```ini
lib_deps =
  https://code.aliyun.com/hacklab/aliyun-mqtt-arduino.git

# Customize PubSub MQTT configs for Aliyun MQTT broker
build_flags =
  -D MQTT_MAX_PACKET_SIZE=256
  -D MQTT_KEEPALIVE=60
```
