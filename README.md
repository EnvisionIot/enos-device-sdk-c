# Using EnOS Device SDK for C

[TOC]

## Installation

### Compiling the SDK

To compile the SDK, you need to install a set of tools depending on the platform you are doing your development on and the one you are targeting. You will also need to clone the current repository.

How to set up the development environment for the C SDK on [Ubuntu](http://www.ubuntu.com/desktop) is described as follows:

1. Make sure all dependencies are installed before building the SDK. For Ubuntu, you can use `apt-get` to install the required packages:

    ```shell
    sudo apt-get update
    sudo apt-get install -y build-essential make git gcc
    ```

2. Clone the latest release of SDK to your local machine:

    ```shell
    git clone https://github.com/EnvisionIot/enos-iot-mqtt-c-sdk.git
    ```

### Building the SDK

Use the following commands to build the SDK:

```shell
cd enos-iot-mqtt-c-sdk
make menuconfig   # customize needed features by kconf
make reconfig     # select configuration for Ubuntu
make
```

There are many feature configuration options available for building the SDK. For example, you can activate the **dynamic activation** feature by enabling it in menuconfig.

You can also select other pre-built environments with the following command:

```shell
make reconfig
SELECT A CONFIGURATION:

1) config.macos.x86
2) config.ubuntu.x86
3) config.windows.x86
#?
```

## Feature List

For the list of features supported by this SDK and the availability of EnOS device connectivity and management features in all SDKs we provide, see [EnOS Device SDK](https://github.com/EnvisionIot/enos-iot-device-sdk).

## Quick Start

1. Connect to EnOS Cloud via an MQTT connection

```c
/* ProductKey, DeviceKey and DeviceSecrect can be obtained in Device Details page in EnOS Console */
char g_product_key[IOTX_PRODUCT_KEY_LEN + 1] = "PRODUCT_KEY";
char g_device_key[IOTX_DEVICE_KEY_LEN + 1] = "DEVICE_KEY";
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1] = "DEVICE_SECRET";

iotx_enos_dev_meta_info_t master_meta_info;
memset(&g_user_example_ctx, 0, sizeof(user_example_ctx_t));

memset(&master_meta_info, 0, sizeof(iotx_enos_dev_meta_info_t));
memcpy(master_meta_info.product_key, g_product_key, strlen(g_product_key));
memcpy(master_meta_info.device_key, g_device_key, strlen(g_device_key));
memcpy(master_meta_info.device_secret, g_device_secret, strlen(g_device_secret));

/* BROKER_URL is the URL of EnOS MQTT Broker for Devices, which can be obtained in Environment Information page in EnOS Console */
char mqtt_uri[50] = "BROKER_URL";
IOT_Ioctl(IOTX_IOCTL_SET_MQTT_DOMAIN, (void *)mqtt_uri);

int mqtt_port = 11883;
IOT_Ioctl(IOTX_IOCTL_SET_MQTT_PORT, (void *)&mqtt_port);

do {
    g_user_example_ctx.master_devid = IOT_EnOS_Open(IOTX_ENOS_DEV_TYPE_MASTER, &master_meta_info);
    if (g_user_example_ctx.master_devid >= 0)
    {
        break;
    }
    EXAMPLE_TRACE("IOT_EnOS_Open failed! retry after %d ms\n", 2000);
    HAL_SleepMs(2000);
} while (1);
do {
    res = IOT_EnOS_Connect(g_user_example_ctx.master_devid);
    if (res >= 0)
    {
        break;
    }
    EXAMPLE_TRACE("IOT_EnOS_Connect failed! retry after %d ms\n", 5000); HAL_SleepMs(5000);
} while (1);
```

2. Report measurement points

```c
void user_sample_post_measurepoint(void)
{
    static int cnt = 0;
    int res = 0;
    char measurepoint_payload[50] = {0};
    /* MeasurePoint1 is a measurepoint defined in ThingModel. */
    HAL_Snprintf(measurepoint_payload, sizeof(measurepoint_payload), "{\"measurepoints\":{\"MeasurePoint1\":%d}}", cnt++);
    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_POST_MEASUREPOINT, (unsigned char *)measurepoint_payload, strlen(measurepoint_payload));
    EXAMPLE_TRACE("Post Measurepoint Message ID: %d, payload: %s", res, measurepoint_payload);
}

while (1)
{
    IOT_EnOS_Yield(EXAMPLE_YIELD_TIMEOUT_MS);

    user_sample_post_measurepoint();

    cnt++;

    if (auto_quit == 1 && cnt > 3600)
    {
        break;
    }
}
```

3. Close the MQTT connection

```c
IOT_EnOS_Close(g_user_example_ctx.master_devid);
```

## Sample Codes

* [Establishing Connection with EnOS Cloud](src/dev_model/examples/enos_example_solo.c)
* [Device Tags](src/dev_model/examples/enos_example_device_tag.c)
* [Device Attributes](src/dev_model/examples/enos_example_device_attribute.c)
* [Reporting Measurement Points](src/dev_model/examples/enos_example_device_measurepoint.c)
* [Reporting Events](src/dev_model/examples/enos_example_device_event.c)
* [Receiving Commands from Cloud](src/dev_model/examples/enos_example_command.c)
* [Passing Through Device Information or Receiving Passed-through Information from Cloud](src/dev_model/examples/enos_example_raw.c)

## Related Information

* To learn more about EnOS IoT Hub, see [EnOS IoT Hub Documentation](https://support.envisioniot.com/docs/device-connection/en/latest/device_management_overview.html).
* To learn more about how to develop your device for EnOS IoT Hub, see [EnOS Device Development Guide (C)]().

## API Reference

Under development

## Release Notes

* 2020/01/08(Initial Release): Support direct connection to EnOS Cloud, bi-directional authentication and dynamic activation.
