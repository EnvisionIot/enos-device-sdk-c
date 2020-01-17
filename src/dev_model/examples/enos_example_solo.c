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

#define EXAMPLE_TRACE(...)                                      \
    do                                                          \
    {                                                           \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__); \
        HAL_Printf(__VA_ARGS__);                                \
        HAL_Printf("\033[0m\r\n");                              \
    } while (0)

#define EXAMPLE_MASTER_DEVID (0)
#define EXAMPLE_YIELD_TIMEOUT_MS (10000)

typedef struct
{
    int master_devid;
    int cloud_connected;
    int master_initialized;
} user_example_ctx_t;

/* direct login */
char g_product_key[IOTX_PRODUCT_KEY_LEN + 1] = "TXepVmTh";
char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "bHGsUshy72T";
char g_device_key[IOTX_DEVICE_KEY_LEN + 1] = "M1Ia3PPDim";
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1] = "t9cLPzFvQc8bPECCSkZ3";

/* dynamic active */
/*
char g_product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "TXepVmTh";
char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "bHGsUshy72T";
char g_device_key[IOTX_DEVICE_KEY_LEN + 1]       = "EgJ5ByekGy";
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "";
 */

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

const char* device_secret = NULL;
static int dynamic_activate_device_secret(const char* ds)
{
    if(ds != NULL && strlen(ds) > 0)
    {
        EXAMPLE_TRACE("******ds: %s******\n", ds);
        device_secret = ds;
        return 0;
    }
    else
    {
        EXAMPLE_TRACE("******dynamic activate failed, no ds received!\n******");
        return -1;
    }
}


void user_sample_post_measurepoint(void);
void user_sample_post_measurepoint_batch(void);

void user_sample_trigger_event(void);

int user_measurepoint_set_event_handler(const int devid, const char *request, const int request_len,
                                        void *p_measurepoint_set_ctx);
int user_service_invoke_event_handler(int devid, const char *serviceid, int serviceid_len,
                                       const char *request, int request_len, void *p_service_ctx);


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

static int do_user_sample(void (*sample)(void), int count, int frequency)
{
    if (count < 0 || frequency <= 0)
    {
        return -1;
    }
    if ((count % frequency) == 0)
    {
        sample();
    }
    return 0;
}


static void do_user_sample_measurepoint()
{
    user_sample_post_measurepoint();
    user_sample_post_measurepoint_batch();
}

static void do_user_sample_event()
{
    user_sample_trigger_event();
}

static void do_user_sample_command()
{
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, user_service_invoke_event_handler);
    IOT_RegisterCallback(ITE_MEASUREPOINT_SET, user_measurepoint_set_event_handler);
}

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
    IOT_RegisterCallback(ITE_DYNAMIC_ACTIVATE_DEVICE_SECRET, dynamic_activate_device_secret);

    do_user_sample_command();

    char mqtt_uri[50] = "beta-iot-as-mqtt-cn4.eniot.io";
    IOT_Ioctl(IOTX_IOCTL_SET_MQTT_DOMAIN, (void *)mqtt_uri);

    int mqtt_port = 11883;
    IOT_Ioctl(IOTX_IOCTL_SET_MQTT_PORT, (void *)&mqtt_port);

    /* post reply doesn't need */
    post_reply_need = 1;
    IOT_Ioctl(IOTX_IOCTL_RECV_THING_REPLY, (void *)&post_reply_need);

    dynamic_activate = 1;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_ACTIVATE, (void *)&dynamic_activate);

    do
    {
        g_user_example_ctx.master_devid = IOT_EnOS_Open(IOTX_ENOS_DEV_TYPE_MASTER, &master_meta_info);
        if (g_user_example_ctx.master_devid >= 0)
        {
            break;
        }
        EXAMPLE_TRACE("IOT_EnOS_Open failed! retry after %d ms\n", 2000);
        HAL_SleepMs(2000);
    } while (1);

    do
    {
        res = IOT_EnOS_Connect(g_user_example_ctx.master_devid);
        if (res >= 0)
        {
            break;
        }
        EXAMPLE_TRACE("IOT_EnOS_Connect failed! retry after %d ms\n", 5000);
        HAL_SleepMs(5000);
    } while (1);

    while (1)
    {
        IOT_EnOS_Yield(EXAMPLE_YIELD_TIMEOUT_MS);

        do_user_sample(do_user_sample_measurepoint, cnt, 2);

        do_user_sample(do_user_sample_event, cnt, 120);

        cnt++;

        if (auto_quit == 1 && cnt > 3600)
        {
            break;
        }
    }

    IOT_EnOS_Close(g_user_example_ctx.master_devid);

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_SetLogLevel(IOT_LOG_NONE);

    return 0;
}

/**
 * tag sample
 */
static char tag_key[20] = "tag";
static char tag_value[20] = "new value";

void user_sample_query_tag(void)
{
    int res = 0;
    cJSON *root = cJSON_CreateObject();

    cJSON *tags = cJSON_CreateArray();
    cJSON_AddItemToArray(tags, cJSON_CreateString(tag_key));
    cJSON_AddItemToObject(root, "tags", tags);

    char *payload = cJSON_Print(root);

    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_QUERY_TAG, (unsigned char *)payload, strlen(payload));
    EXAMPLE_TRACE("Query Tags, msgid: %d", res);

    cJSON_Delete(root);
    cJSON_free(payload);
}

void user_sample_update_tag(void)
{
    static int cnt = 0;
    int res = 0;
    cJSON *root = cJSON_CreateArray();

    cJSON *tag1 = cJSON_CreateObject();
    char digit = '0' + cnt;
    memcpy(tag_key, "tag", strlen("tag"));
    //memcpy(tag_key + strlen(tag_key), &digit, 1);
    memcpy(tag_value, "new value", strlen("new value"));
    //memcpy(tag_value + strlen(tag_value), &digit, 1);
    cJSON_AddItemToObject(tag1, "tagKey", cJSON_CreateString(tag_key));
    cJSON_AddItemToObject(tag1, "tagValue", cJSON_CreateString(tag_value));

    printf("*****************tag_key: %s,  tag_value: %s***************\n", tag_key, tag_value);

    cJSON_AddItemToArray(root, tag1);

    char *payload = cJSON_Print(root);

    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_UPDATE_TAG, (unsigned char *)payload, strlen(payload));
    EXAMPLE_TRACE("Update Tags, msgid: %d", res);

    cJSON_Delete(root);
    cJSON_free(payload);

    cnt++;
    if (cnt > 10)
    {
        cnt = 0;
    }
}

void user_sample_delete_tag(void)
{
    int res = 0;
    cJSON *root = cJSON_CreateObject();

    cJSON *tags = cJSON_CreateArray();
    cJSON_AddItemToArray(tags, cJSON_CreateString(tag_key));
    cJSON_AddItemToObject(root, "tags", tags);

    char *payload = cJSON_Print(root);

    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_DELETE_TAG, (unsigned char *)payload, strlen(payload));
    EXAMPLE_TRACE("Delete Tags, msgid: %d", res);

    cJSON_Delete(root);
    cJSON_free(payload);
}

/**
 * attribute sample
 */
void user_sample_query_attribute(void) {
    int res = 0;

    char attribute_payload[50] = {0};
    HAL_Snprintf(attribute_payload, sizeof(attribute_payload), "{\"attributes\":[\"1.0\"]}");

    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_QUERY_ATTRIBUTE, (unsigned char*)attribute_payload, strlen(attribute_payload));

    EXAMPLE_TRACE("Query Attribute Message ID: %d, payload: %s", res, attribute_payload);
}


void user_sample_update_attribute(void) {
    static int cnt = 1;
    int res = 0;
    char attribute_payload[50] = {5};
    HAL_Snprintf(attribute_payload, sizeof(attribute_payload), "{\"attributes\":{\"1.0\": %d}}", cnt++);
    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_UPDATE_ATTRIBUTE, (unsigned char *)attribute_payload, strlen(attribute_payload));

    EXAMPLE_TRACE("Update Attribute Message ID: %d, payload: %s", res, attribute_payload);
}


void user_sample_delete_attribute(void) {
    int res = 0;
    char attribute_payload[50] = {0};
    HAL_Snprintf(attribute_payload, sizeof(attribute_payload), "{\"attributes\":[\"维吾尔\"]}");

    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_DELETE_ATTRIBUTE, (unsigned char*)attribute_payload, strlen(attribute_payload));

    EXAMPLE_TRACE("Delete Attribute Message ID: %d, payload: %s", res, attribute_payload);
    EXAMPLE_TRACE("Delete Attribute Message ID: %d", res);
}

/**
 * event sample
 */
void user_sample_trigger_event(void)
{
    int res = 0;
    char *event_id = "Error";
    char *event_payload = "{\"events\": {\"Error\": 0} }";

    res = IOT_EnOS_TriggerEvent(EXAMPLE_MASTER_DEVID, event_id, strlen(event_id),
                                   event_payload, strlen(event_payload));
    EXAMPLE_TRACE("Post Event Message ID: %d", res);
}

/**
 * measurepoint sample
 */
void user_sample_post_measurepoint(void)
{
    static int cnt = 0;
    int res = 0;

    char measurepoint_payload[50] = {0};
    HAL_Snprintf(measurepoint_payload, sizeof(measurepoint_payload), "{\"measurepoints\":{\"mp1\":%d}}", cnt++);

    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_POST_MEASUREPOINT,
                          (unsigned char *)measurepoint_payload, strlen(measurepoint_payload));

    EXAMPLE_TRACE("Post Measurepoint Message ID: %d, payload: %s", res, measurepoint_payload);
}

void user_sample_post_measurepoint_batch(void)
{
    static int cnt = 0;
    int res, i = 0;

    cJSON *root = NULL;

    root = cJSON_CreateArray();


        cJSON *item = cJSON_CreateObject();

        cJSON *measurepoint = cJSON_CreateObject();
        cJSON_AddNumberToObject(measurepoint, "mp1", ++cnt);
        cJSON_AddNumberToObject(measurepoint, "mp2", cnt);
        cJSON_AddItemToObject(item, "measurepoints", measurepoint);

        cJSON_AddNumberToObject(item, "time", HAL_UptimeMs() * 1000 + i * 500);

        cJSON_AddItemReferenceToArray(root, item);


    char *out = cJSON_Print(root);

    EXAMPLE_TRACE("batch post measurepoint payload: %s", out);

    char measurepoint_payload[200] = {0};
    memcpy(measurepoint_payload, out, strlen(out));

    cJSON_Delete(root);

    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_POST_MEASUREPOINT_BATCH,
                          (unsigned char *)measurepoint_payload, strlen(measurepoint_payload));

    EXAMPLE_TRACE("Post Measurepoint Message ID: %d, payload: %s", res, measurepoint_payload);
}

/**
 * measurepoint resume sample
 */
void user_sample_resume_measurepoint(void)
{
    static int cnt = 0;
    int res = 0;

    char measurepoint_payload[50] = {0};
    HAL_Snprintf(measurepoint_payload, sizeof(measurepoint_payload), "{\"measurepoints\":{\"mp1\":%d}}", cnt++);

    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_RESUME_MEASUREPOINT,
                          (unsigned char *)measurepoint_payload, strlen(measurepoint_payload));

    EXAMPLE_TRACE("Post Measurepoint Message ID: %d, payload: %s", res, measurepoint_payload);
}

void user_sample_resume_measurepoint_batch(void)
{
    static int cnt = 0;
    int res, i = 0;

    cJSON *root = NULL;

    root = cJSON_CreateArray();

    // for (i = 0; i < 2; i++)
    // {
        cJSON *item = cJSON_CreateObject();

        cJSON *measurepoint = cJSON_CreateObject();
        cJSON_AddNumberToObject(measurepoint, "mp1", cnt);
        cJSON_AddNumberToObject(measurepoint, "mp2", cnt);
        cJSON_AddItemToObject(item, "measurepoints", measurepoint);

        cJSON_AddNumberToObject(item, "time", HAL_UptimeMs() * 1000 + i * 500);

        cJSON_AddItemReferenceToArray(root, item);
    // }

    char *out = cJSON_Print(root);

    EXAMPLE_TRACE("batch post measurepoint payload: %s", out);

    char measurepoint_payload[200] = {0};
    memcpy(measurepoint_payload, out, strlen(out));

    cJSON_Delete(root);

    res = IOT_EnOS_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_RESUME_MEASUREPOINT_BATCH,
                          (unsigned char *)measurepoint_payload, strlen(measurepoint_payload));

    EXAMPLE_TRACE("Post Measurepoint Message ID: %d, payload: %s", res, measurepoint_payload);
}

/**
 * command sample
 */
/** recv measurepoint setting message from cloud **/
int user_measurepoint_set_event_handler(const int devid, const char *request, const int request_len,
           void* p_measurepoint_set_ctx)
{
    int res = 0;
    EXAMPLE_TRACE("Measurepoint Set Received, Request: %s", request);

    res = IOT_EnOS_CommandReply(EXAMPLE_MASTER_DEVID, IOTX_COMMAND_MEASUREPOINT_SET,
            NULL, 0, "{}", 2, p_measurepoint_set_ctx);
    EXAMPLE_TRACE("Set Measurepoint Message ID: %d", res);

    return 0;
}


int user_service_invoke_event_handler(int devid, const char *serviceid, int serviceid_len,
                                                  const char *request, int request_len, void *p_service_ctx)
{
    int add_result = 0;
    cJSON *root = NULL, *item_number_a = NULL, *item_number_b = NULL;
    const char *response_fmt = "{\"Result\": %d}";
    char response[30] = {0};
    int response_len = 0;

    EXAMPLE_TRACE("Service Request Received, Service ID: %.*s, Payload: %s", serviceid_len, serviceid, request);

    /* Parse Root */
    root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root)) {
        EXAMPLE_TRACE("JSON Parse Error");
        return -1;
    }

    if (strlen("Operation_Service") == serviceid_len && memcmp("Operation_Service", serviceid, serviceid_len) == 0) {
        /* Parse NumberA */
        item_number_a = cJSON_GetObjectItem(root, "NumberA");
        if (item_number_a == NULL || !cJSON_IsNumber(item_number_a)) {
            cJSON_Delete(root);
            return -1;
        }
        EXAMPLE_TRACE("NumberA = %d", item_number_a->valueint);

        /* Parse NumberB */
        item_number_b = cJSON_GetObjectItem(root, "NumberB");
        if (item_number_b == NULL || !cJSON_IsNumber(item_number_b)) {
            cJSON_Delete(root);
            return -1;
        }
        EXAMPLE_TRACE("NumberB = %d", item_number_b->valueint);

        add_result = item_number_a->valueint + item_number_b->valueint;

        /* Send Service Response To Cloud immediately */
        HAL_Snprintf(response, sizeof(response), response_fmt, add_result);
        response_len = strlen(response);
        IOT_EnOS_CommandReply(devid, IOTX_COMMAND_SERVICE_INVOKE,
                (char *)serviceid, serviceid_len, response, response_len, p_service_ctx);
    }

    cJSON_Delete(root);
    return 0;
}
