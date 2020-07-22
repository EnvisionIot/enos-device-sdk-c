#include "iotx_dm_internal.h"

static dm_client_uri_map_t g_dm_client_uri_map[] = {
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
    /* EnOS Command - Set Measurepoint */
    {DM_URI_THING_MEASUREPOINT_SET,             DM_URI_SYS_PREFIX,             IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_measurepoint_set             },

    /* EnOS Command - Invoke Service */
    {DM_URI_THING_SERVICE_INVOKE_WILDCARD,      DM_URI_SYS_PREFIX,             IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_service_invoke              },

    /* EnOS Tags */
    {DM_URI_THING_TAG_QUERY_REPLY,              DM_URI_SYS_PREFIX,             IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_reply             },
    {DM_URI_THING_TAG_UPDATE_REPLY,             DM_URI_SYS_PREFIX,             IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_reply             },
    {DM_URI_THING_TAG_DELETE_REPLY,             DM_URI_SYS_PREFIX,             IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_reply             },

    /* EnOS ThingModel Attributes */
    {DM_URI_THING_ATTRIBUTE_QUERY_REPLY,        DM_URI_SYS_PREFIX,             IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_reply             },
    {DM_URI_THING_ATTRIBUTE_UPDATE_REPLY,       DM_URI_SYS_PREFIX,             IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_reply             },
    {DM_URI_THING_ATTRIBUTE_DELETE_REPLY,       DM_URI_SYS_PREFIX,             IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_reply             },

    /* EnOS ThingModel Measurepoints */
    {DM_URI_THING_MEASUREPOINT_POST_REPLY,          DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_reply             },
    {DM_URI_THING_MEASUREPOINT_POST_BATCH_REPLY,    DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_reply             },

#ifdef DEVICE_MEASUREPOINT_RESUME
    /* EnOS ThingModel Measurepoints Resume */
    {DM_URI_THING_MEASUREPOINT_RESUME_REPLY,        DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_reply             },
    {DM_URI_THING_MEASUREPOINT_RESUME_BATCH_REPLY,  DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_reply             },
#endif /* #ifdef DEVICE_MEASUREPOINT_RESUME */

    /* EnOS ThingModel Events */
    {DM_URI_THING_EVENT_POST_REPLY_WILDCARD,    DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_reply             },
#endif
    {DM_URI_THING_MODEL_DOWN_RAW,               DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_model_down_raw               },
    {DM_URI_THING_MODEL_UP_RAW_REPLY,           DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_ALL, (void *)dm_client_thing_model_up_raw_reply           },

#ifdef DEVICE_MODEL_GATEWAY
    {DM_URI_THING_DEVICE_REGISTER_REPLY,        DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_device_register_reply        },
    
    {DM_URI_THING_TOPO_ADD_REPLY,               DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_topo_add_reply               },
    {DM_URI_THING_TOPO_DELETE_REPLY,            DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_topo_delete_reply            },
    {DM_URI_THING_TOPO_GET_REPLY,               DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_topo_get_reply               },
    {DM_URI_COMBINE_LOGIN_REPLY,                DM_URI_EXT_SESSION_PREFIX, IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_combine_login_reply                },
    {DM_URI_COMBINE_LOGIN_BATCH_REPLY,          DM_URI_EXT_SESSION_PREFIX, IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_combine_login_batch_reply          },
    {DM_URI_COMBINE_LOGOUT_REPLY,               DM_URI_EXT_SESSION_PREFIX, IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_combine_logout_reply               },
    {DM_URI_THING_DISABLE,                      DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_disable                      },
    {DM_URI_THING_ENABLE,                       DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_enable                       },
    {DM_URI_THING_DELETE,                       DM_URI_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_thing_delete                       },
    {DM_URI_COMBINE_DISABLE,                    DM_URI_EXT_SESSION_PREFIX, IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_combine_disable                    },
    {DM_URI_COMBINE_ENABLE,                     DM_URI_EXT_SESSION_PREFIX, IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_combine_enable                     },
    {DM_URI_COMBINE_DELETE,                     DM_URI_EXT_SESSION_PREFIX, IOTX_DM_DEVICE_GATEWAY, (void *)dm_client_combine_delete                     },
#endif
};

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
/* measurepoint/event post reply filter */
static int _dm_client_subscribe_filter(char *uri, iotx_cm_data_handle_cb cb)
{
    int res = 0;
    int event_post_reply_opt = 0;
    int retry_cnt = IOTX_DM_CLIENT_SUB_RETRY_MAX_COUNTS;

    res = dm_opt_get(DM_OPT_DOWNSTREAM_THING_REPLY, &event_post_reply_opt);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    if (event_post_reply_opt == 0) {
        res = dm_client_unsubscribe(uri);
        return res;
    } else {
        res = -1;
        while (res < SUCCESS_RETURN && retry_cnt--) {
            res = dm_client_subscribe(uri, cb, 0);
        }
        return res;
    }
}
#endif /* #if !defined(DEVICE_MODEL_RAWDATA_SOLO) */

int dm_client_subscribe_all(int devid, char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                            char device_key[IOTX_DEVICE_KEY_LEN + 1],
                            int dev_type)
{
    int res = 0, index = 0;
    int number = sizeof(g_dm_client_uri_map) / sizeof(dm_client_uri_map_t);
    char *uri = NULL;
    uint8_t local_sub = 0;

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
    /* index 0 must be DM_URI_THING_EVENT_POST_REPLY_WILDCARD */
    res = dm_utils_service_name((char *)g_dm_client_uri_map[0].uri_prefix, (char *)g_dm_client_uri_map[0].uri_name,
                                product_key, device_key, &uri);
    if (res == SUCCESS_RETURN) {
        _dm_client_subscribe_filter(uri, (iotx_cm_data_handle_cb)g_dm_client_uri_map[0].callback);
        DM_free(uri);
    }
    index = 1;
#else
    index = 0;
#endif

#ifdef MQTT_AUTO_SUBSCRIBE
    if (devid != 0) {
        iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_IN_AUTOSUB_MODE, "subdev subscribe bypass");
        return SUCCESS_RETURN;
    }
#else
    (void)devid;
#endif  /* #ifdef MQTT_AUTO_SUBSCRIBE */

    for (; index < number; index++) {
        if ((g_dm_client_uri_map[index].dev_type & dev_type) == 0) {
            continue;
        }

#ifdef MQTT_AUTO_SUBSCRIBE
        res = dm_utils_service_name((char *)g_dm_client_uri_map[index].uri_prefix, (char *)g_dm_client_uri_map[index].uri_name,
                                    "+", "+", &uri);    /* plus sign wildcards used */
        if (res < SUCCESS_RETURN) {
            continue;
        }

        local_sub = 1;
        res = dm_client_subscribe(uri, (iotx_cm_data_handle_cb)g_dm_client_uri_map[index].callback, &local_sub);
        DM_free(uri);
#else
        res = dm_utils_service_name((char *)g_dm_client_uri_map[index].uri_prefix, (char *)g_dm_client_uri_map[index].uri_name,
                                    product_key, device_key, &uri);
        if (res < SUCCESS_RETURN) {
            continue;
        }

        {
            int retry_cnt = IOTX_DM_CLIENT_SUB_RETRY_MAX_COUNTS;
            local_sub = 0;
            do {
                res = dm_client_subscribe(uri, (iotx_cm_data_handle_cb)g_dm_client_uri_map[index].callback, &local_sub);
            } while (res < SUCCESS_RETURN && --retry_cnt);
            DM_free(uri);
        }
#endif /*  MQTT_AUTO_SUBSCRIBE */
    }

    return SUCCESS_RETURN;
}

static void _dm_client_event_cloud_connected_handle(void)
{
    dm_msg_cloud_connected();
}

static void _dm_client_event_cloud_disconnect_handle(void)
{
    dm_msg_cloud_disconnect();
}

void dm_client_event_handle(int fd, iotx_cm_event_msg_t *event, void *context)
{
    switch (event->type) {
        case IOTX_CM_EVENT_CLOUD_CONNECTED: {
            _dm_client_event_cloud_connected_handle();
        }
        break;
        case IOTX_CM_EVENT_CLOUD_CONNECT_FAILED: {

        }
        break;
        case IOTX_CM_EVENT_CLOUD_DISCONNECT: {
            _dm_client_event_cloud_disconnect_handle();
        }
        break;
        default:
            break;
    }
}

void dm_client_thing_model_down_raw(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_model_down_raw(&source);
}

void dm_client_thing_model_up_raw_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                        void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_model_up_raw_reply(&source);
}
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
void dm_client_thing_measurepoint_set(int fd, const char *topic, const char *payload, unsigned int payload_len,
        void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;
    int measurepoint_set_auto_reply_opt = 0;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_THING_MEASUREPOINT_SET_REPLY;

    res = dm_msg_proc_thing_measurepoint_set(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    measurepoint_set_auto_reply_opt = 0;
    res = dm_opt_get(DM_OPT_UPSTREAM_MEASUREPOINT_SET_AUTO_REPLY, &measurepoint_set_auto_reply_opt);
    if (res == SUCCESS_RETURN) {
        if (measurepoint_set_auto_reply_opt) {
            dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
        }
    }
}

void dm_client_thing_service_invoke(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                     void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_service_invoke(&source);
}

void dm_client_thing_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                      void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_reply(&source);
}
#endif

#ifdef DEVICE_MODEL_GATEWAY
int dm_client_subdev_unsubscribe(char product_key[IOTX_PRODUCT_KEY_LEN + 1], char device_key[IOTX_DEVICE_KEY_LEN + 1])
{
    int res = 0, index = 0;
    int number = sizeof(g_dm_client_uri_map) / sizeof(dm_client_uri_map_t);
    char *uri = NULL;

    for (index = 0; index < number; index++) {
        if ((g_dm_client_uri_map[index].dev_type & IOTX_DM_DEVICE_SUBDEV) == 0) {
            continue;
        }

        res = dm_utils_service_name((char *)g_dm_client_uri_map[index].uri_prefix, (char *)g_dm_client_uri_map[index].uri_name,
                                    product_key, device_key, &uri);
        if (res < SUCCESS_RETURN) {
            index--;
            continue;
        }

        dm_client_unsubscribe(uri);
        DM_free(uri);
    }

    return SUCCESS_RETURN;
}

void dm_client_thing_disable(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_THING_DISABLE_REPLY;

    res = dm_msg_proc_thing_disable(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
}

void dm_client_thing_enable(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_THING_ENABLE_REPLY;

    res = dm_msg_proc_thing_enable(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
}

void dm_client_thing_delete(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_THING_DELETE_REPLY;

    res = dm_msg_proc_thing_delete(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
}

void dm_client_combine_disable(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_COMBINE_DISABLE_REPLY;

    res = dm_msg_proc_combine_disable(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
}

void dm_client_combine_enable(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_COMBINE_ENABLE_REPLY;

    res = dm_msg_proc_combine_enable(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
}

void dm_client_combine_delete(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context)
{
    int res = 0;
    dm_msg_source_t source;
    dm_msg_dest_t dest;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&source, 0, sizeof(dm_msg_source_t));
    memset(&dest, 0, sizeof(dm_msg_dest_t));
    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dest.uri_name = DM_URI_COMBINE_DELETE_REPLY;

    res = dm_msg_proc_combine_delete(&source, &dest, &request, &response);
    if (res < SUCCESS_RETURN) {
        return;
    }

    dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, "{}", strlen("{}"), NULL);
}


void dm_client_thing_device_register_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                        void *context)
{
    
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_device_register_reply(&source);
}

void dm_client_thing_topo_add_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_topo_add_reply(&source);
}

void dm_client_thing_topo_delete_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                       void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_topo_delete_reply(&source);
}

void dm_client_thing_topo_get_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_thing_topo_get_reply(&source);
}

void dm_client_combine_login_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                   void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_combine_login_reply(&source);
}

void dm_client_combine_login_batch_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                   void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_combine_login_batch_reply(&source);
}

void dm_client_combine_logout_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context)
{
    dm_msg_source_t source;

    memset(&source, 0, sizeof(dm_msg_source_t));

    source.uri = topic;
    source.payload = (unsigned char *)payload;
    source.payload_len = payload_len;
    source.context = NULL;

    dm_msg_proc_combine_logout_reply(&source);
}
#endif
