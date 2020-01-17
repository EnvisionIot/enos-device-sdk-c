#include <stdio.h>
#include <string.h>
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "dynamic_activate_api.h"
#include "wrappers.h"

char g_product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "TXepVmTh";
char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "bHGsUshy72T";
char g_device_name[IOTX_DEVICE_KEY_LEN + 1]        = "EgJ5ByekGy";
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "";

int main(int argc, char *argv[])
{
    int32_t res = 0;
    iotx_dev_meta_info_t meta;
    iotx_mqtt_region_types_t region = IOTX_CLOUD_REGION_CUSTOM;

    HAL_Printf("dynamic activate example\n");



    char mqtt_uri[50] = "beta-iot-as-mqtt-cn4.eniot.io";
    IOT_Ioctl(IOTX_IOCTL_SET_MQTT_DOMAIN, (void *)mqtt_uri);

    int mqtt_port = 11883;
    IOT_Ioctl(IOTX_IOCTL_SET_MQTT_PORT, (void *) &mqtt_port);


    memset(&meta, 0, sizeof(iotx_dev_meta_info_t));
    memcpy(meta.product_key, g_product_key, strlen(g_product_key));
    memcpy(meta.product_secret, g_product_secret, strlen(g_product_secret));
    memcpy(meta.device_key, g_device_name, strlen(g_device_name));

    res = IOT_Dynamic_Activate(region, &meta);
    if (res < 0) {
        HAL_Printf("IOT_Dynamic_Activate failed\n");
        return -1;
    }

    HAL_Printf("\nDevice Secret: %s\n\n", meta.device_secret);

    return 0;
}
