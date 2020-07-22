/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */
#include "infra_config.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "cJSON.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "infra_log.h"
#include "infra_compat.h"
#include "infra_log.h"
#include "dev_model_api.h"
// #include "dm_wrapper.h"

#ifdef ENOS_GATEWAY_TEST_CMD
#include "simulate_subdev/testcmd.h"
#endif

#if defined(OTA_ENABLED) && defined(BUILD_AOS)
#include "ota_service.h"
#endif

static char g_product_key[IOTX_PRODUCT_KEY_LEN + 1] = "gKaFNsHb";
static char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "ZxqX0jLPGui";
static char g_device_key[IOTX_DEVICE_KEY_LEN + 1] = "c-sdk-gw";
static char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1] = "9ITfAST9ZDwkxdh6zQaH";

#define USER_EXAMPLE_YIELD_TIMEOUT_MS (200)

#define EXAMPLE_TRACE(...)                                      \
    do                                                          \
    {                                                           \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__); \
        HAL_Printf(__VA_ARGS__);                                \
        HAL_Printf("\033[0m\r\n");                              \
    } while (0)

#define EXAMPLE_SUBDEV_ADD_NUM 2
#define EXAMPLE_SUBDEV_MAX_NUM 5
const iotx_enos_dev_meta_info_t subdev_arr[EXAMPLE_SUBDEV_MAX_NUM] =
    {
        {"WQcKt75h",
         "KjKRibddgFm",
         "c-sdk-subdev",
         "yK2akuLjiDAeG2NRLxQw"},
        {"WQcKt75h",
         "KjKRibddgFm",
         "nHNJV2hFtT",
         "vroBC8msQJSF9ZRK2fq9"}};

typedef struct
{
    int auto_add_subdev;
    int master_devid;
    int cloud_connected;
    int master_initialized;
    int subdev_index;
    int permit_join;
    void *g_user_dispatch_thread;
    int g_user_dispatch_thread_running;
} user_example_ctx_t;

static user_example_ctx_t g_user_example_ctx;

void *example_malloc(size_t size)
{
    return HAL_Malloc(size);
}

void example_free(void *ptr)
{
    HAL_Free(ptr);
}

static user_example_ctx_t *user_example_get_ctx(void)
{
    return &g_user_example_ctx;
}

static int user_connected_event_handler(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("Cloud Connected");

    user_example_ctx->cloud_connected = 1;

    return 0;
}

static int user_disconnected_event_handler(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("Cloud Disconnected");

    user_example_ctx->cloud_connected = 0;

    return 0;
}

static int user_initialized(const int devid)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    EXAMPLE_TRACE("Device Initialized, Devid: %d", devid);

    if (user_example_ctx->master_devid == devid)
    {
        user_example_ctx->master_initialized = 1;
        user_example_ctx->subdev_index++;
    }

    return 0;
}

static uint64_t user_update_sec(void)
{
    static uint64_t time_start_ms = 0;

    if (time_start_ms == 0)
    {
        time_start_ms = HAL_UptimeMs();
    }

    return (HAL_UptimeMs() - time_start_ms) / 1000;
}

void user_post_measurepoint(int devid)
{
    int res = 0;
    static int cnt = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    char measurepoint_payload[50] = {0};
    HAL_Snprintf(measurepoint_payload, sizeof(measurepoint_payload), "{\"measurepoints\":{\"mp1\":%d}}", cnt++);

    res = IOT_EnOS_Report(devid, ITM_MSG_POST_MEASUREPOINT,
                          (unsigned char *)measurepoint_payload, strlen(measurepoint_payload));
    EXAMPLE_TRACE("Post Property Message ID: %d", res);
}

static int user_master_dev_available(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    if (user_example_ctx->cloud_connected && user_example_ctx->master_initialized)
    {
        return 1;
    }

    return 0;
}

/* #ifdef DEVICE_MODEL_GATEWAY */

static int example_subdev_login_batch()
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    int res = 0, index = 0;
    int devid;
    int devids[EXAMPLE_SUBDEV_ADD_NUM];

    for (index = 0; index < EXAMPLE_SUBDEV_ADD_NUM; index++)
    {
        devid = IOT_EnOS_Open(IOTX_ENOS_DEV_TYPE_SLAVE, (iotx_enos_dev_meta_info_t *)&subdev_arr[index]);
        if (devid == FAIL_RETURN)
        {
            EXAMPLE_TRACE("subdev open Failed\n");
            return FAIL_RETURN;
        }

        devids[index] = devid;
    }

    res = IOT_EnOS_Report(user_example_ctx->master_devid, ITM_MSG_LOGIN_BATCH,
                          (unsigned char *)devids, EXAMPLE_SUBDEV_ADD_NUM);

    return res;
}

static int user_example_measurepoint_post_batch()
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    static int cnt = 0;
    int res, index = 0;

    char measurepoint_payload[1024] = {0};
    char *out;

    cJSON *root = cJSON_CreateArray();

    cJSON *item;
    cJSON *measurepoint;

    for (index = 0; index < EXAMPLE_SUBDEV_ADD_NUM; index++)
    {
        item = cJSON_CreateObject();

        cJSON_AddStringToObject(item, "productKey", subdev_arr[index].product_key);
        cJSON_AddStringToObject(item, "deviceKey", subdev_arr[index].device_key);

        measurepoint = cJSON_CreateObject();
        cJSON_AddNumberToObject(measurepoint, "mp_int", cnt++);
        cJSON_AddItemToObject(item, "measurepoints", measurepoint);

        cJSON_AddNumberToObject(item, "time", HAL_UptimeMs() * 1000 + index * 500);

        cJSON_AddItemReferenceToArray(root, item);
    }

    out = cJSON_Print(root);
    EXAMPLE_TRACE("batch post measurepoint payload: %s", out);
    memcpy(measurepoint_payload, out, strlen(out));

    /* free json */
    cJSON_Delete(root);
    cJSON_free(out);

    res = IOT_EnOS_Report(user_example_ctx->master_devid, ITM_MSG_POST_MEASUREPOINT_BATCH,
                          (unsigned char *)measurepoint_payload, strlen(measurepoint_payload));

    EXAMPLE_TRACE("Post Measurepoint Message ID: %d, payload: %s", res, measurepoint_payload);

    return res;
}

/* #endif */

/** recv enos reply message from cloud **/
static int user_thing_reply_event_handler(const int devid, const int msgid, const int code,
                                          const char *method, const int method_len,
                                          const char *data, const int data_len)
{
    EXAMPLE_TRACE("EnOS Reply Received, ID: %d, Code: %d, Method: %.*s, Data: %.*s",
                  msgid, code, method_len, method, data_len,
                  (data == NULL) ? ("(null)") : data);

    return 0;
}

void *user_dispatch_yield(void *args)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    while (user_example_ctx->g_user_dispatch_thread_running)
    {
        IOT_EnOS_Yield(USER_EXAMPLE_YIELD_TIMEOUT_MS);
    }

    return NULL;
}

static int max_running_seconds = 0;
int main(int argc, char **argv)
{
    int res = 0;
    uint64_t time_prev_sec = 0, time_now_sec = 0, time_begin_sec = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    iotx_enos_dev_meta_info_t master_meta_info;
    int domain_type = 0;
    int dynamic_activate = 0;
    int post_reply_need = 0;

    memset(user_example_ctx, 0, sizeof(user_example_ctx_t));

#if defined(__UBUNTU_SDK_DEMO__)
    if (argc > 1)
    {
        int tmp = atoi(argv[1]);

        if (tmp >= 60)
        {
            max_running_seconds = tmp;
            EXAMPLE_TRACE("set [max_running_seconds] = %d seconds\n", max_running_seconds);
        }
    }

    if (argc > 2)
    {
        if (strlen("auto") == strlen(argv[2]) &&
            memcmp("auto", argv[2], strlen(argv[2])) == 0)
        {
            user_example_ctx->auto_add_subdev = 1;
        }
    }
#endif

    user_example_ctx->subdev_index = -1;

    IOT_SetLogLevel(IOT_LOG_DEBUG);

    /* Register Callback */
    IOT_RegisterCallback(ITE_CONNECT_SUCC, user_connected_event_handler);
    IOT_RegisterCallback(ITE_DISCONNECTED, user_disconnected_event_handler);
    IOT_RegisterCallback(ITE_THING_REPLY, user_thing_reply_event_handler);
    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, user_initialized);

    memset(&master_meta_info, 0, sizeof(iotx_enos_dev_meta_info_t));
    memcpy(master_meta_info.product_key, g_product_key, strlen(g_product_key));
    memcpy(master_meta_info.product_secret, g_product_secret, strlen(g_product_secret));
    memcpy(master_meta_info.device_key, g_device_key, strlen(g_device_key));
    memcpy(master_meta_info.device_secret, g_device_secret, strlen(g_device_secret));

    /* Create Master Device Resources */
    user_example_ctx->master_devid = IOT_EnOS_Open(IOTX_ENOS_DEV_TYPE_MASTER, &master_meta_info);
    if (user_example_ctx->master_devid < 0)
    {
        EXAMPLE_TRACE("IOT_EnOS_Open Failed\n");
        return -1;
    }

    char mqtt_uri[50] = "beta-iot-as-mqtt-cn4.eniot.io";
    IOT_Ioctl(IOTX_IOCTL_SET_MQTT_DOMAIN, (void *)mqtt_uri);

    int mqtt_port = 11883;
    IOT_Ioctl(IOTX_IOCTL_SET_MQTT_PORT, (void *)&mqtt_port);

    /* post reply doesn't need */
    post_reply_need = 1;
    IOT_Ioctl(IOTX_IOCTL_RECV_THING_REPLY, (void *)&post_reply_need);

    do
    {
        res = IOT_EnOS_Connect(user_example_ctx->master_devid);
        if (res < 0)
        {
            EXAMPLE_TRACE("IOT_EnOS_Connect failed, retry after 5s...\n");
            HAL_SleepMs(5000);
        }
    } while (res < 0);

    user_example_ctx->g_user_dispatch_thread_running = 1;
    res = HAL_ThreadCreate(&user_example_ctx->g_user_dispatch_thread, user_dispatch_yield, NULL, NULL, NULL);
    if (res < 0)
    {
        EXAMPLE_TRACE("HAL_ThreadCreate Failed\n");
        IOT_EnOS_Close(user_example_ctx->master_devid);
        return -1;
    }

    example_subdev_login_batch();

    user_example_ctx->auto_add_subdev = 1;

    time_begin_sec = user_update_sec();
    while (1)
    {
        HAL_SleepMs(200);

        time_now_sec = user_update_sec();
        if (time_prev_sec == time_now_sec)
        {
            continue;
        }
        if (max_running_seconds && (time_now_sec - time_begin_sec > max_running_seconds))
        {
            EXAMPLE_TRACE("Example Run for Over %d Seconds, Break Loop!\n", max_running_seconds);
            break;
        }

        if (user_master_dev_available())
        {
            user_example_measurepoint_post_batch();
        }

        /* Post Measurepoint Example */
        if (time_now_sec % 11 == 0 && user_master_dev_available())
        {
            /* user_post_measurepoint(user_example_ctx->master_devid); */
        }

        time_prev_sec = time_now_sec;
    }

    user_example_ctx->g_user_dispatch_thread_running = 0;
    IOT_EnOS_Close(user_example_ctx->master_devid);
    /*HAL_ThreadDelete(user_example_ctx->g_user_dispatch_thread);*/

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_SetLogLevel(IOT_LOG_NONE);
    return 0;
}
