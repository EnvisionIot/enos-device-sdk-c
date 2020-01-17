/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */
#include <stdio.h>
#include "string.h"
#include "infra_state.h"
#include "infra_compat.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_string.h"
#include "infra_sha256.h"
#include "dynamic_activate_internal.h"
#include "dynamic_activate_api.h"
#ifdef DYNAMIC_ACTIVATE
    #include "dev_sign_api.h"
    #include "mqtt_api.h"
#endif

#define HTTP_RESPONSE_PAYLOAD_LEN           (256)

#define DYNAMIC_ACTIVATE_RANDOM_KEY_LENGTH            (15)
#define DYNAMIC_ACTIVATE_SIGN_LENGTH                  (65)
#define DYNAMIC_ACTIVATE_SIGN_METHOD_HMACSHA256       "hmacsha256"

#ifdef DYNAMIC_ACTIVATE
static int _mqtt_dynamic_activate_sign_password(iotx_dev_meta_info_t *meta_info, iotx_sign_mqtt_t *signout, char *timestamp)
{
    char signsource[DEV_SIGN_SOURCE_MAXLEN] = {0};
    uint16_t signsource_len = 0;
    const char sign_fmt[] = "clientId%sdeviceKey%sproductKey%stimestamp%s%s";
    uint8_t sign_hex[32] = {0};

    char device_id[64] = {0};
    memcpy(device_id, meta_info->product_key, strlen(meta_info->product_key));
    memcpy(device_id + strlen(device_id), ".", strlen("."));
    memcpy(device_id + strlen(device_id), meta_info->device_key, strlen(meta_info->device_key));

    signsource_len = strlen(sign_fmt) + strlen(meta_info->device_key) + 1 + strlen(meta_info->product_key) + 
                     strlen(timestamp) + 1;
    if (signsource_len >= DEV_SIGN_SOURCE_MAXLEN) {
        return STATE_MQTT_SIGN_SOURCE_BUF_SHORT;
    }
    memset(signsource, 0, signsource_len);
    memcpy(signsource, "clientId", strlen("clientId"));
    memcpy(signsource + strlen(signsource), device_id, strlen(device_id));
    memcpy(signsource + strlen(signsource), "deviceKey", strlen("deviceKey"));
    memcpy(signsource + strlen(signsource), meta_info->device_key, strlen(meta_info->device_key));
    memcpy(signsource + strlen(signsource), "productKey", strlen("productKey"));
    memcpy(signsource + strlen(signsource), meta_info->product_key, strlen(meta_info->product_key));
    memcpy(signsource + strlen(signsource), "timestamp", strlen("timestamp"));
    memcpy(signsource + strlen(signsource), timestamp, strlen(timestamp));
    memcpy(signsource + strlen(signsource), meta_info->product_secret, strlen(meta_info->product_secret));

                      
    utils_sha256((uint8_t *)signsource, strlen(signsource), sign_hex);

    /* utils_shm_sha256((uint8_t *)signsource, strlen(signsource), (uint8_t *)device_secret,
                      strlen(device_secret), sign_hex); */
    infra_hex2str(sign_hex, 32, signout->password);

    return STATE_SUCCESS;
}

static int32_t _mqtt_dynamic_activate_sign_clientid(iotx_dev_meta_info_t *meta_info, iotx_sign_mqtt_t *signout, char *rand)
{
    const char * clientid = "|securemode=3,signmethod=sha256,timestamp=";
    uint32_t clientid_len = 0;

    char device_id[64] = {0};
    memcpy(device_id, meta_info->product_key, strlen(meta_info->product_key));
    memcpy(device_id + strlen(device_id), ".", strlen("."));
    memcpy(device_id + strlen(device_id), meta_info->device_key, strlen(meta_info->device_key));

    clientid_len = strlen(meta_info->product_key) + 1 + strlen(meta_info->device_key) + 1;
    if (clientid_len >= DEV_SIGN_CLIENT_ID_MAXLEN) {
        return ERROR_DEV_SIGN_CLIENT_ID_TOO_SHORT;
    }
    memset(signout->clientid, 0, clientid_len);
    memcpy(signout->clientid, device_id, strlen(device_id));
    memcpy(signout->clientid + strlen(signout->clientid), clientid, strlen(clientid));
    memcpy(signout->clientid + strlen(signout->clientid), rand, strlen(rand));
    memcpy(signout->clientid + strlen(signout->clientid), "|", 1);

    return STATE_SUCCESS;
}

static void find_method_pos(const char *topic, int topic_len, const char **start_pos) {
    int count = 5;
    const char *ptopic = topic;
    while(ptopic < topic + topic_len) {
        if (*ptopic == '/') {
            count--;
        }
        if (count == 0) {
            *start_pos = ptopic;
            break;
        }
        ptopic++;
    }
}

void _mqtt_dynamic_activate_topic_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    int32_t res = 0;
    char *ds = (char *)pcontext;
    iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    const char *asterisk = "**********************";

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED: {
            /* print topic name and topic message */
            char *device_secret = NULL;
            uint32_t device_secret_len = 0;
            const char* method_start_pos = 0;

            find_method_pos(topic_info->ptopic, strlen(topic_info->ptopic), &method_start_pos);

            if (memcmp(method_start_pos, "/thing/activate/info", strlen("/thing/activate/info"))) {
                return;
            }

            /* parse secret */
            res = infra_json_value((char *)topic_info->payload, topic_info->payload_len, "deviceSecret", strlen("deviceSecret"),
                                   &device_secret, &device_secret_len);
            if (res == STATE_SUCCESS) {
                memcpy(ds, device_secret + 1, device_secret_len - 2);
                memcpy(device_secret + 1 + 5, asterisk, strlen(asterisk));
                dynamic_activate_info("Topic  : %.*s", topic_info->topic_len, topic_info->ptopic);
                dynamic_activate_info("Payload: %.*s", topic_info->payload_len, topic_info->payload);
            }
        }
        break;
        default:
            break;
    }
}

int32_t _mqtt_dynamic_activate(iotx_mqtt_region_types_t region, iotx_dev_meta_info_t *meta)
{
    void *pClient = NULL;
    iotx_mqtt_param_t mqtt_params;
    int32_t res = 0;
    uint32_t length = 0;
    uint64_t timestamp = 0;
    char *timestamp_str = "2524608000000";
    iotx_sign_mqtt_t signout;
    uint64_t timestart = 0, timenow = 0;
    char device_secret[IOTX_DEVICE_SECRET_LEN + 1] = {0};

    memset(&signout, 0, sizeof(iotx_sign_mqtt_t));

    /* setup hostname */
    if (IOTX_CLOUD_REGION_CUSTOM == region) {
        if (g_infra_mqtt_domain[region] == NULL) {
            return STATE_USER_INPUT_MQTT_DOMAIN;
        }

        length = strlen(g_infra_mqtt_domain[region]) + 1;
        if (length >= DEV_SIGN_HOSTNAME_MAXLEN) {
            return STATE_MQTT_SIGN_HOSTNAME_BUF_SHORT;
        }

        memset(signout.hostname, 0, DEV_SIGN_HOSTNAME_MAXLEN);
        memcpy(signout.hostname, g_infra_mqtt_domain[region], strlen(g_infra_mqtt_domain[region]));
    } else {
        length = strlen(meta->product_key) + strlen(g_infra_mqtt_domain[region]) + 2;
        if (length >= DEV_SIGN_HOSTNAME_MAXLEN) {
            return STATE_MQTT_SIGN_HOSTNAME_BUF_SHORT;
        }
        memset(signout.hostname, 0, DEV_SIGN_HOSTNAME_MAXLEN);
        memcpy(signout.hostname, meta->product_key, strlen(meta->product_key));
        memcpy(signout.hostname + strlen(signout.hostname), ".", strlen("."));
        memcpy(signout.hostname + strlen(signout.hostname), g_infra_mqtt_domain[region],
               strlen(g_infra_mqtt_domain[region]));
    }

    /* setup port */
    /* signout.port = 443; */

    /* reset port to enos mqtt protocol port */
#ifdef SUPPORT_TLS
    signout.port = 18883;
#else
    signout.port = 11883;
#endif


   /*  if (g_sdk_impl_ctx.mqtt_port_num != 0) {
        ((iotx_mqtt_param_t *)_mqtt_conncection->open_params)->port = g_sdk_impl_ctx.mqtt_port_num;
    } */

    /* setup username */
    length = strlen(meta->device_key) + strlen(meta->product_key) + 2;
    if (length >= DEV_SIGN_USERNAME_MAXLEN) {
        return STATE_MQTT_SIGN_USERNAME_BUF_SHORT;
    }
    memset(signout.username, 0, DEV_SIGN_USERNAME_MAXLEN);
    memcpy(signout.username, meta->device_key, strlen(meta->device_key));
    memcpy(signout.username + strlen(signout.username), "&", strlen("&"));
    memcpy(signout.username + strlen(signout.username), meta->product_key, strlen(meta->product_key));

    /* password */
    res = _mqtt_dynamic_activate_sign_password(meta, &signout, timestamp_str);
    if (res < 0) {
        return res;
    }

    /* client id */
    res = _mqtt_dynamic_activate_sign_clientid(meta, &signout, timestamp_str);
    if (res < 0) {
        return res;
    }

    memset(&mqtt_params, 0, sizeof(iotx_mqtt_param_t));
    mqtt_params.host = signout.hostname;
    mqtt_params.port = signout.port;
    mqtt_params.username = signout.username;
    mqtt_params.password = signout.password;
    mqtt_params.client_id = signout.clientid;
    mqtt_params.request_timeout_ms = 5000;
    mqtt_params.clean_session = 1;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.read_buf_size = 1000;
    mqtt_params.write_buf_size = 1000;
    mqtt_params.handle_event.h_fp = _mqtt_dynamic_activate_topic_handle;
    mqtt_params.handle_event.pcontext = device_secret;

#ifdef SUPPORT_TLS
    {
        extern const char *iotx_ca_crt;
        mqtt_params.pub_key = iotx_ca_crt;
    }
#endif

    pClient =  wrapper_mqtt_init(&mqtt_params);
    if (pClient == NULL) {
        return STATE_MQTT_WRAPPER_INIT_FAIL;
    }

    res = wrapper_mqtt_connect(pClient);
    if (res < STATE_SUCCESS) {
        wrapper_mqtt_release(&pClient);
        return res;
    }

    timestart = HAL_UptimeMs();
    while (1) {
        timenow = HAL_UptimeMs();
        if (timenow < timestart) {
            timestart = timenow;
        }

        if (timestart - timenow >= MQTT_DYNAMIC_ACTIVATE_TIMEOUT_MS) {
            break;
        }

        wrapper_mqtt_yield(pClient, 200);

        if (strlen(device_secret) > 0) {
            break;
        }
    }

    if (strlen(device_secret) > 0) {
        res = STATE_SUCCESS;
    } else {
        res = STATE_MQTT_DYNAMIC_ACTIVATE_FAIL_RESP;
    }

    wrapper_mqtt_release(&pClient);

    memcpy(meta->device_secret, device_secret, strlen(device_secret));

    return res;
}
#endif

int32_t IOT_Dynamic_Activate(iotx_mqtt_region_types_t region, iotx_dev_meta_info_t *meta)
{
    return _mqtt_dynamic_activate(region, meta);
}

