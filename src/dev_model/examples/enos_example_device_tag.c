/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */
#include "infra_config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __UBUNTU_SDK_DEMO__
#include <unistd.h>
#endif

#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "infra_log.h"
#ifdef INFRA_MEM_STATS
#include "infra_mem_stats.h"
#endif
#include "dev_model_api.h"
#include "wrappers.h"
#include "cJSON.h"
#ifdef ATM_ENABLED
#include "at_api.h"
#endif

static int response_id;
static int response_code;
static int request_id;

#define EXAMPLE_TRACE(...)                                      \
    do                                                          \
    {                                                           \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__); \
        HAL_Printf(__VA_ARGS__);                                \
        HAL_Printf("\033[0m\r\n");                              \
    } while (0)

#define EXAMPLE_MASTER_DEVID (0)
#define EXAMPLE_YIELD_TIMEOUT_MS (1000)

typedef struct
{
    int master_devid;
    int cloud_connected;
    int master_initialized;
} user_example_ctx_t;

static user_example_ctx_t g_user_example_ctx;

/** cloud connected event callback */
static int user_connected_event_handler(void)
{
    EXAMPLE_TRACE("Cloud Connected");
    g_user_example_ctx.cloud_connected = 1;

    return 0;
}

/** cloud disconnected event callback */
static int user_disconnected_event_handler(void)
{
    EXAMPLE_TRACE("Cloud Disconnected");
    g_user_example_ctx.cloud_connected = 0;

    return 0;
}

/* device initialized event callback */
static int user_initialized(const int devid)
{
    EXAMPLE_TRACE("Device Initialized");
    g_user_example_ctx.master_initialized = 1;

    return 0;
}

/** recv enos reply message from cloud **/
static int user_thing_reply_event_handler(const int devid, const int msgid, const int code,
                                          const char *method, const int method_len,
                                          const char *data, const int data_len)
{
    EXAMPLE_TRACE("EnOS Reply Received, ID: %d, Code: %d, Method: %.*s, Data: %.*s",
                  msgid, code, method_len, method, data_len,
                  (data == NULL) ? ("(null)") : data);
    response_id = msgid;
    response_code = code;
    return 0;
}

static int user_cloud_error_handler(const int code, const char *data, const char *detail)
{
    EXAMPLE_TRACE("code =%d ,data=%s, detail=%s", code, data, detail);
    return 0;
}

static int user_sdk_state_dump(int ev, const char *msg)
{
    printf("received state: -0x%04X(%s)\n", -ev, msg);
    return 0;
}

static char tag_key[20] = "tag";
static char tag_value[20] = "new value";

static void user_sample_query_tag(void)
{
    int res = 0;
    cJSON *root = cJSON_CreateObject();

    cJSON *tags = cJSON_CreateArray();
    cJSON_AddItemToArray(tags, cJSON_CreateString(tag_key));
    cJSON_AddItemToObject(root, "tags", tags);

    char *payload = cJSON_Print(root);

    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_QUERY_TAG, (unsigned char *)payload, strlen(payload));
    EXAMPLE_TRACE("Query Tags, msgid: %d", res);
    request_id = res;

    cJSON_Delete(root);
    cJSON_free(payload);
}

static void user_sample_update_tag(void)
{
    static int cnt = 0;
    int res = 0;
    cJSON *root = cJSON_CreateArray();

    cJSON *tag1 = cJSON_CreateObject();
    char digit = '0' + cnt;
    memcpy(tag_key, "tag", strlen("tag"));
    memcpy(tag_value, "new value", strlen("new value"));
    cJSON_AddItemToObject(tag1, "tagKey", cJSON_CreateString(tag_key));
    cJSON_AddItemToObject(tag1, "tagValue", cJSON_CreateString(tag_value));

    printf("*****************tag_key: %s,  tag_value: %s***************\n", tag_key, tag_value);

    cJSON_AddItemToArray(root, tag1);

    char *payload = cJSON_Print(root);

    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_UPDATE_TAG, (unsigned char *)payload, strlen(payload));
    EXAMPLE_TRACE("Update Tags, msgid: %d", res);

    request_id = res;

    cJSON_Delete(root);
    cJSON_free(payload);

    cnt++;
    if (cnt > 10)
    {
        cnt = 0;
    }
}

static void user_sample_delete_tag(void)
{
    int res = 0;
    cJSON *root = cJSON_CreateObject();

    cJSON *tags = cJSON_CreateArray();
    cJSON_AddItemToArray(tags, cJSON_CreateString(tag_key));
    cJSON_AddItemToObject(root, "tags", tags);

    char *payload = cJSON_Print(root);

    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_DELETE_TAG, (unsigned char *)payload, strlen(payload));
    EXAMPLE_TRACE("Delete Tags, msgid: %d", res);
    request_id = res;

    cJSON_Delete(root);
    cJSON_free(payload);
}

static char g_product_key[IOTX_PRODUCT_KEY_LEN + 1] = "product_key";
static char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "product_secret";
static char g_device_key[IOTX_DEVICE_KEY_LEN + 1] = "device_key";
static char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1] = "device_secret";

int main(int argc, char **argv)
{
    int res = 0;
    int cnt = 0;
    int auto_quit = 0;
    iotx_enos_dev_meta_info_t master_meta_info;
    int dynamic_activate = 0, post_reply_need = 0;

    if (argc >= 2 && !strcmp("auto_quit", argv[1]))
    {
        auto_quit = 1;
        cnt = 0;
    }
    memset(&g_user_example_ctx, 0, sizeof(user_example_ctx_t));

    memset(&master_meta_info, 0, sizeof(iotx_enos_dev_meta_info_t));
    memcpy(master_meta_info.product_key, g_product_key, strlen(g_product_key));
    memcpy(master_meta_info.product_secret, g_product_secret, strlen(g_product_secret));
    memcpy(master_meta_info.device_key, g_device_key, strlen(g_device_key));
    memcpy(master_meta_info.device_secret, g_device_secret, strlen(g_device_secret));

    IOT_SetLogLevel(IOT_LOG_DEBUG);

    /* Register Callback */
    IOT_RegisterCallback(ITE_STATE_EVERYTHING, user_sdk_state_dump);
    IOT_RegisterCallback(ITE_CONNECT_SUCC, user_connected_event_handler);
    IOT_RegisterCallback(ITE_DISCONNECTED, user_disconnected_event_handler);

    IOT_RegisterCallback(ITE_THING_REPLY, user_thing_reply_event_handler);

    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, user_initialized);
    IOT_RegisterCallback(ITE_CLOUD_ERROR, user_cloud_error_handler);

    char mqtt_uri[50] = "mqtt_domain_url";
    IOT_Ioctl(IOTX_IOCTL_SET_MQTT_DOMAIN, (void *)mqtt_uri);

    int mqtt_port = 11883;
    IOT_Ioctl(IOTX_IOCTL_SET_MQTT_PORT, (void *)&mqtt_port);

    /* post reply doesn't need */
    post_reply_need = 1;
    IOT_Ioctl(IOTX_IOCTL_RECV_THING_REPLY, (void *)&post_reply_need);

    dynamic_activate = 0;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_ACTIVATE, (void *)&dynamic_activate);

    do
    {
        g_user_example_ctx.master_devid = IOT_EnOS_Open(IOTX_ENOS_DEV_TYPE_MASTER, &master_meta_info);
        if (g_user_example_ctx.master_devid >= 0)
        {
            break;
        }
    } while (1);

    /* run_ubuntu_wifi_provision_example(); */
    do
    {
        res = IOT_EnOS_Connect(g_user_example_ctx.master_devid);
        if (res >= 0)
        {
            break;
        }
    } while (1);

    user_sample_update_tag();
    IOT_EnOS_Yield(EXAMPLE_YIELD_TIMEOUT_MS);
    if (response_id == request_id && response_code == 200)
    {
        EXAMPLE_TRACE("******user_sample_update_tag success, request_id: %d, response_id: %d, response_code: %d******\n",
                      request_id, response_id, response_code);
    }
    else
    {
        EXAMPLE_TRACE("******user_sample_update_tag failed, request_id: %d, response_id: %d, response_code: %d******\n",
                      request_id, response_id, response_code);
    }

    user_sample_query_tag();
    IOT_EnOS_Yield(EXAMPLE_YIELD_TIMEOUT_MS);
    if (response_id == request_id && response_code == 200)
    {
        EXAMPLE_TRACE("******user_sample_query_tag success, request_id: %d, response_id: %d, response_code: %d******\n",
                      request_id, response_id, response_code);
    }
    else
    {
        EXAMPLE_TRACE("******user_sample_query_tag failed, request_id: %d, response_id: %d, response_code: %d******\n",
                      request_id, response_id, response_code);
    }

    user_sample_delete_tag();
    IOT_EnOS_Yield(EXAMPLE_YIELD_TIMEOUT_MS);
    if (response_id == request_id && response_code == 200)
    {
        EXAMPLE_TRACE("******user_sample_delete_tag success, request_id: %d, response_id: %d, response_code: %d******\n",
                      request_id, response_id, response_code);
    }
    else
    {
        EXAMPLE_TRACE("******user_sample_delete_tag failed, request_id: %d, response_id: %d, response_code: %d******\n",
                      request_id, response_id, response_code);
    }

    IOT_EnOS_Close(g_user_example_ctx.master_devid);

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_SetLogLevel(IOT_LOG_NONE);


    return 0;
}
