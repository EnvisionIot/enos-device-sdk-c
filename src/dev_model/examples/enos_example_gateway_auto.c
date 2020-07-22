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

static char g_product_key[IOTX_PRODUCT_KEY_LEN + 1] = "gateway_product_key";
static char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "gateway_product_secret";
static char g_device_key[IOTX_DEVICE_KEY_LEN + 1] = "gateway_device_key";
static char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1] = "gateway_device_secret";

#define USER_EXAMPLE_YIELD_TIMEOUT_MS (200)

#define EXAMPLE_TRACE(...)                                      \
    do                                                          \
    {                                                           \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__); \
        HAL_Printf(__VA_ARGS__);                                \
        HAL_Printf("\033[0m\r\n");                              \
    } while (0)


#define REGISTER_DEVICE_NUM_MAX 10
static int register_success_device_num = 0;
static int register_failure_device_num = 0;

static int new_device_devid_arr[REGISTER_DEVICE_NUM_MAX] = {-1};
static int new_device_devid_num = 0;


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
    HAL_Snprintf(measurepoint_payload, sizeof(measurepoint_payload), "{\"measurepoints\":{\"mp_int\":%d}}", cnt++);

    res = IOT_EnOS_Report(devid, ITM_MSG_POST_MEASUREPOINT,
                          (unsigned char *)measurepoint_payload, strlen(measurepoint_payload));
    EXAMPLE_TRACE("Post Measurepoint Message ID: %d", res);
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

int is_succeed(int code)
{
    return code == 0 || code == 200 ? 1 : 0;
}


static int user_register_device_reply_hanlder(const int devid, const int msgid, const int code)
{
    int res = 0;

    if (is_succeed(code)) {
        register_success_device_num++;
        new_device_devid_arr[new_device_devid_num++] = devid;
        res = IOT_EnOS_Report(devid, ITM_MSG_ADD_TOPO, NULL, 0);
        if (res == FAIL_RETURN)
        {
            EXAMPLE_TRACE("login device failed, devid = %d\n", devid);
            return res;
        }
        res = IOT_EnOS_Report(devid, ITM_MSG_LOGIN, NULL, 0);
        if (res == FAIL_RETURN)
        {
            EXAMPLE_TRACE("login device failed, devid = %d\n", devid);
            return res;
        }
    } else {
        register_failure_device_num++;
        EXAMPLE_TRACE("register device failed, code = %d\n", code);
    }

    return res;
}

/* #ifdef DEVICE_MODEL_GATEWAY */
static int example_register_device()
{
    static int new_device_devid = 0;

    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    
    char register_info[200] = {0};

    new_device_devid++;
    if (new_device_devid >= REGISTER_DEVICE_NUM_MAX)
    {
        EXAMPLE_TRACE("over max register device number");
        return res;
    }
    HAL_Snprintf(register_info, sizeof(register_info), 
        "[{\"timezone\":\"+08:00\", \"productKey\":\"subdevice_product_key\", \"deviceKey\":\"c-sdk-test-register%d\", \"deviceName\":{\"defaultValue\":\"c-sdk-test-register%d\"}}]",
        new_device_devid, new_device_devid);

    res = IOT_EnOS_Report(user_example_ctx->master_devid, ITM_MSG_REGISTER, 
                            (unsigned char *)register_info, strlen(register_info));
    if (res >= 0) 
    {
        EXAMPLE_TRACE("Register device Message ID: %d\n", res);
    }
    return res;
}

static int example_add_topo()
{
    int res = 0, index = 0, devid = -1;

    if (new_device_devid_num <= 0) 
    {
        EXAMPLE_TRACE("There is not device for add topo");
        return res;
    }

    for (index = 0; index < new_device_devid_num; index ++)
    {
        devid = new_device_devid_arr[index];
        res = IOT_EnOS_Report(devid, ITM_MSG_ADD_TOPO, NULL, 0);
        if (res == FAIL_RETURN)
        {
            EXAMPLE_TRACE("add topo Failed, sub-devid = %d\n", devid);
        }
    }
    return index;
}

static int example_batch_login()
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    res = IOT_EnOS_Report(user_example_ctx->master_devid, ITM_MSG_LOGIN_BATCH,
                         (unsigned char *)new_device_devid_arr, new_device_devid_num);
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

static int user_thing_disable_handler(const int devid)
{
    EXAMPLE_TRACE("Receive Device disable msg, devid = %d\n", devid);
    return 0;
}

static int user_thing_enable_handler(const int devid)
{
    EXAMPLE_TRACE("Receive Device enable msg, devid = %d\n", devid);
    return 0;
}
static int user_thing_delete_handler(const int devid, const char*product_key, const int product_key_len, const char*device_key, const int device_key_len)
{
    EXAMPLE_TRACE("Receive Device delete msg, devid = %d, produc_key = %.*s, device_key = %.*s\n", devid, product_key_len, product_key, device_key_len, device_key);
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

    IOT_RegisterCallback(ITE_DEVICE_REGISTER_REPLY, user_register_device_reply_hanlder);
    IOT_RegisterCallback(ITE_COMBINE_DISABLE, user_thing_disable_handler);
    IOT_RegisterCallback(ITE_COMBINE_ENABLE, user_thing_enable_handler);
    IOT_RegisterCallback(ITE_COMBINE_DELETE, user_thing_delete_handler);

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

    char mqtt_uri[50] = "mqtt_domain_url";
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

    example_register_device(); 

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

        /* Post Proprety Example */
        if (time_now_sec % 11 == 0 && user_master_dev_available())
        {
            user_post_measurepoint(new_device_devid_arr[0]);
        }

        /* Device Info Update Example */
        if (time_now_sec % 23 == 0 && user_master_dev_available())
        {
            /* user_deviceinfo_update(); */
        }

        /* Device Info Delete Example */
        if (time_now_sec % 29 == 0 && user_master_dev_available())
        {
            /* user_deviceinfo_delete(); */
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
