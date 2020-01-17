/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */

#include "iotx_dm_internal.h"

#if defined(DEVICE_MODEL_ENABLED)
#include "dev_model_api.h"
#include "dm_opt.h"

#ifdef INFRA_MEM_STATS
    #include "infra_mem_stats.h"
    #define IMPL_ENOS_MALLOC(size)            LITE_malloc(size, MEM_MAGIC, "impl.enos")
    #define IMPL_ENOS_FREE(ptr)               LITE_free(ptr)
#else
    #define IMPL_ENOS_MALLOC(size)            HAL_Malloc(size)
    #define IMPL_ENOS_FREE(ptr)               {HAL_Free((void *)ptr);ptr = NULL;}
#endif

#define IOTX_ENOS_KEY_ID          "id"
#define IOTX_ENOS_KEY_CODE        "code"
#define IOTX_ENOS_KEY_DEVID       "devid"
#define IOTX_ENOS_KEY_SERVICEID   "serviceid"
#define IOTX_ENOS_KEY_MEASUREPOINTID  "measurepointid"
#define IOTX_ENOS_KEY_EVENTID     "eventid"
#define IOTX_ENOS_KEY_PAYLOAD     "payload"
#define IOTX_ENOS_KEY_CONFIG_ID   "configId"
#define IOTX_ENOS_KEY_CONFIG_SIZE "configSize"
#define IOTX_ENOS_KEY_GET_TYPE    "getType"
#define IOTX_ENOS_KEY_SIGN        "sign"
#define IOTX_ENOS_KEY_SIGN_METHOD "signMethod"
#define IOTX_ENOS_KEY_URL         "url"
#define IOTX_ENOS_KEY_VERSION     "version"
#define IOTX_ENOS_KEY_UTC         "utc"
#define IOTX_ENOS_KEY_CTX         "ctx"
#define IOTX_ENOS_KEY_TOPO        "topo"
#define IOTX_ENOS_KEY_PRODUCT_KEY "productKey"
#define IOTX_ENOS_KEY_TIME        "time"
#define IOTX_ENOS_KEY_DATA        "data"
#define IOTX_ENOS_KEY_MESSAGE     "message"

#define IOTX_ENOS_SYNC_DEFAULT_TIMEOUT_MS    10000

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
typedef enum {
    IOTX_SERVICE_REQ_TYPE_GENERAL,
} iotx_service_req_type_t;

typedef struct {
    void *service_ctx;
} service_meta_general_t;

typedef struct {
    /* int msgid; */
    char* msgid;
    int msgid_len;
    int type;
    union {
        service_meta_general_t general;
    } service_meta;
    uint64_t ctime;
    struct list_head linked_list;
} iotx_command_ctx_node_t;
#endif /* #if !defined(DEVICE_MODEL_RAWDATA_SOLO) */

typedef struct {
    int msgid;
    void *semaphore;
    int code;
    struct list_head linked_list;
} iotx_enos_upstream_sync_callback_node_t;

typedef struct {
    void *mutex;
    void *upstream_mutex;
    void *command_list_mutex;
    int command_list_num;
    int is_opened;
    int is_connected;
    int is_yield_running;
    int yield_running;
    struct list_head upstream_sync_callback_list;
    struct list_head downstream_command_list;
} iotx_enos_ctx_t;

static iotx_msg_type_method_map_t g_msg_type_method_map[] = {
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
    {ITM_MSG_QUERY_TAG, "thing.tag.query"},
    {ITM_MSG_UPDATE_TAG, "thing.tag.update"},
    {ITM_MSG_DELETE_TAG, "thing.tag.delete"},

    {ITM_MSG_QUERY_ATTRIBUTE, "thing.attribute.query"},
    {ITM_MSG_UPDATE_ATTRIBUTE, "thing.attribute.update"},
    {ITM_MSG_DELETE_ATTRIBUTE, "thing.attribute.delete"},

#ifdef DEVICE_MEASUREPOINT_RESUME
    {ITM_MSG_RESUME_MEASUREPOINT, "thing.measurepoint.resume"},
    {ITM_MSG_RESUME_MEASUREPOINT_BATCH, "thing.measurepoint.resume.batch"},
#endif

    {ITM_MSG_POST_MEASUREPOINT, "thing.measurepoint.post"},
    {ITM_MSG_POST_MEASUREPOINT_BATCH, "thing.measurepoint.post.batch"},
#endif
};

static char* iotx_enos_get_method(iotx_enos_msg_type_t msg_type)
{
    char* method = NULL;
    int i=0;
    int map_max_size = sizeof(g_msg_type_method_map) / sizeof(iotx_msg_type_method_map_t);
    while (i < map_max_size) {
        if (g_msg_type_method_map[i].msg_type == msg_type) {
            method = g_msg_type_method_map[i].method;
            break;
        }
        i++;
    }
    return method;
}

static iotx_enos_ctx_t g_iotx_enos_ctx = {0};

static iotx_enos_ctx_t *_iotx_enos_get_ctx(void)
{
    return &g_iotx_enos_ctx;
}

static void _iotx_enos_mutex_lock(void)
{
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();
    if (ctx->mutex) {
        HAL_MutexLock(ctx->mutex);
    }
}

static void _iotx_enos_mutex_unlock(void)
{
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();
    if (ctx->mutex) {
        HAL_MutexUnlock(ctx->mutex);
    }
}

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
static void _enos_command_list_mutex_lock(void)
{
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();
    if (ctx->command_list_mutex) {
        HAL_MutexLock(ctx->command_list_mutex);
    }
}

static void _enos_command_list_mutex_unlock(void)
{
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();
    if (ctx->command_list_mutex) {
        HAL_MutexUnlock(ctx->command_list_mutex);
    }
}

static int _enos_command_list_insert(iotx_service_req_type_t type, char *msgid, char *rrpcid, int rrpcid_len,
                                        void *command_ctx, void **p_node)
{
    iotx_command_ctx_node_t *insert_node = NULL;
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();

    if (ctx->command_list_num >= CONFIG_COMMAND_LIST_MAXLEN) {
        iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_INTERNAL_ERROR, "command list full");
        return STATE_DEV_MODEL_INTERNAL_ERROR;
    }

    insert_node = (iotx_command_ctx_node_t *)IMPL_ENOS_MALLOC(sizeof(iotx_command_ctx_node_t));
    if (insert_node == NULL) {
        iotx_state_event(ITE_STATE_DEV_MODEL, STATE_SYS_DEPEND_MALLOC, NULL);
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(insert_node, 0, sizeof(iotx_command_ctx_node_t));

    /* infra_str2int(msgid, &insert_node->msgid); */
    insert_node->msgid_len = 0;
    while (*(msgid + insert_node->msgid_len) >= '0' && *(msgid + insert_node->msgid_len) <= '9') {
        insert_node->msgid_len++;
    }
    insert_node->msgid = IMPL_ENOS_MALLOC(insert_node->msgid_len);
    memcpy(insert_node->msgid, msgid, insert_node->msgid_len);

    insert_node->type = type;
    insert_node->ctime = HAL_UptimeMs();

    if (type == IOTX_SERVICE_REQ_TYPE_GENERAL) {
        insert_node->service_meta.general.service_ctx = command_ctx;
    }

    _enos_command_list_mutex_lock();
    list_add(&insert_node->linked_list, &ctx->downstream_command_list);
    ctx->command_list_num++;
    _enos_command_list_mutex_unlock();

    *p_node = insert_node;
    return SUCCESS_RETURN;
}

static int _enos_command_list_search(iotx_command_ctx_node_t *node)
{
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();
    iotx_command_ctx_node_t *search_node;

    _enos_command_list_mutex_lock();
    list_for_each_entry(search_node, &ctx->downstream_command_list, linked_list, iotx_command_ctx_node_t) {
        if (search_node == node) {
            _enos_command_list_mutex_unlock();
            return SUCCESS_RETURN;
        }
    }
    _enos_command_list_mutex_unlock();

    return STATE_DEV_MODEL_SERVICE_CTX_NOT_EXIST;
}

static int _enos_command_list_delete(iotx_command_ctx_node_t *node)
{
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();

    if (node->type == IOTX_SERVICE_REQ_TYPE_GENERAL) {
        list_del(&node->linked_list);
        IMPL_ENOS_FREE(node->msgid);
        IMPL_ENOS_FREE(node);
    }

    if (ctx->command_list_num > 0) {
        ctx->command_list_num--;
    }

    return SUCCESS_RETURN;
}

static int _enos_service_list_destroy(void)
{
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();
    iotx_command_ctx_node_t *search_node, *temp;

    _enos_command_list_mutex_lock();
    list_for_each_entry_safe(search_node, temp, &ctx->downstream_command_list, linked_list, iotx_command_ctx_node_t) {
        if (search_node->type == IOTX_SERVICE_REQ_TYPE_GENERAL) {
            list_del(&search_node->linked_list);
            IMPL_ENOS_FREE(search_node->msgid);
            IMPL_ENOS_FREE(search_node);
        } else if (search_node->type == IOTX_SERVICE_REQ_TYPE_GENERAL) {
            list_del(&search_node->linked_list);
            IMPL_ENOS_FREE(search_node->msgid);
            IMPL_ENOS_FREE(search_node);
        }
    }
    ctx->command_list_num = 0;
    _enos_command_list_mutex_unlock();

    return SUCCESS_RETURN;
}

void iotx_enos_service_list_overtime_handle(void)
{
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();
    iotx_command_ctx_node_t *search_node, *temp;
    uint64_t current_time = HAL_UptimeMs();

    _enos_command_list_mutex_lock();
    list_for_each_entry_safe(search_node, temp, &ctx->downstream_command_list, linked_list, iotx_command_ctx_node_t) {
        if (current_time < search_node->ctime) {
            search_node->ctime = current_time;
        }
        if (current_time - search_node->ctime >= CONFIG_SERVICE_REQUEST_TIMEOUT) {
            _enos_command_list_delete(search_node);
        }
    }
    _enos_command_list_mutex_unlock();
}
#endif /* #if !defined(DEVICE_MODEL_RAWDATA_SOLO) */

#ifdef DEVICE_MODEL_GATEWAY
static void _iotx_enos_upstream_mutex_lock(void)
{
    iotx_enos_ctx_t *service_ctx = _iotx_enos_get_ctx();
    if (service_ctx->upstream_mutex) {
        HAL_MutexLock(service_ctx->upstream_mutex);
    }
}

static void _iotx_enos_upstream_mutex_unlock(void)
{
    iotx_enos_ctx_t *service_ctx = _iotx_enos_get_ctx();
    if (service_ctx->upstream_mutex) {
        HAL_MutexUnlock(service_ctx->upstream_mutex);
    }
}

static int _iotx_enos_upstream_sync_callback_list_insert(int msgid, void *semaphore,
        iotx_enos_upstream_sync_callback_node_t **node)
{
    iotx_enos_ctx_t *service_ctx = _iotx_enos_get_ctx();
    iotx_enos_upstream_sync_callback_node_t *search_node = NULL;

    list_for_each_entry(search_node, &service_ctx->upstream_sync_callback_list, linked_list,
                        iotx_enos_upstream_sync_callback_node_t) {
        if (search_node->msgid == msgid) {
            return STATE_DEV_MODEL_DUP_UPSTREAM_MSG;
        }
    }

    search_node = IMPL_ENOS_MALLOC(sizeof(iotx_enos_upstream_sync_callback_node_t));
    if (search_node == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(search_node, 0, sizeof(iotx_enos_upstream_sync_callback_node_t));
    search_node->msgid = msgid;
    search_node->semaphore = semaphore;
    INIT_LIST_HEAD(&search_node->linked_list);

    list_add(&search_node->linked_list, &service_ctx->upstream_sync_callback_list);
    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_SYNC_REQ_LIST, "sync list insert, msgid: %d", msgid);

    *node = search_node;
    return SUCCESS_RETURN;
}

static int _iotx_enos_upstream_sync_callback_list_remove(int msgid)
{
    iotx_enos_ctx_t *service_ctx = _iotx_enos_get_ctx();
    iotx_enos_upstream_sync_callback_node_t *search_node = NULL;

    list_for_each_entry(search_node, &service_ctx->upstream_sync_callback_list, linked_list,
                        iotx_enos_upstream_sync_callback_node_t) {
        if (search_node->msgid == msgid) {
            HAL_SemaphoreDestroy(search_node->semaphore);
            list_del(&search_node->linked_list);
            IMPL_ENOS_FREE(search_node);
            return SUCCESS_RETURN;
        }
    }

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_UPSTREAM_REC_NOT_FOUND, "msgid: %d", msgid);
    return STATE_DEV_MODEL_UPSTREAM_REC_NOT_FOUND;
}

static int _iotx_enos_upstream_sync_callback_list_search(int msgid,
        iotx_enos_upstream_sync_callback_node_t **node)
{
    iotx_enos_ctx_t *service_ctx = _iotx_enos_get_ctx();
    iotx_enos_upstream_sync_callback_node_t *search_node = NULL;

    list_for_each_entry(search_node, &service_ctx->upstream_sync_callback_list, linked_list,
                        iotx_enos_upstream_sync_callback_node_t) {
        if (search_node->msgid == msgid) {
            *node = search_node;
            return SUCCESS_RETURN;
        }
    }

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_UPSTREAM_REC_NOT_FOUND, "msgid: %d", msgid);
    return STATE_DEV_MODEL_UPSTREAM_REC_NOT_FOUND;
}

static void _iotx_enos_upstream_sync_callback_list_destroy(void)
{
    iotx_enos_ctx_t *service_ctx = _iotx_enos_get_ctx();
    iotx_enos_upstream_sync_callback_node_t *search_node = NULL, *next_node = NULL;

    list_for_each_entry_safe(search_node, next_node, &service_ctx->upstream_sync_callback_list, linked_list,
                             iotx_enos_upstream_sync_callback_node_t) {
        list_del(&search_node->linked_list);
        HAL_SemaphoreDestroy(search_node->semaphore);
        IMPL_ENOS_FREE(search_node);
    }
}


static void _iotx_enos_upstream_callback_remove(int msgid, int code)
{
    int res = 0;
    iotx_enos_upstream_sync_callback_node_t *sync_node = NULL;
    res = _iotx_enos_upstream_sync_callback_list_search(msgid, &sync_node);
    if (res == SUCCESS_RETURN) {
        sync_node->code = (code == IOTX_DM_ERR_CODE_SUCCESS) ? (SUCCESS_RETURN) : (STATE_DEV_MODEL_REFUSED_BY_CLOUD);
        iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_SYNC_REQ_LIST, "sync list remove, msgid: %d", msgid);
        HAL_SemaphorePost(sync_node->semaphore);
    }
}
#endif

static void _iotx_enos_event_callback(iotx_dm_event_types_t type, char *payload)
{
    int res = 0;
    void *callback;
    lite_cjson_t lite, lite_item_id, lite_item_devid, lite_item_serviceid, lite_item_payload, lite_item_ctx;
    lite_cjson_t lite_item_code, lite_item_eventid, lite_item_utc, lite_item_rrpcid, lite_item_topo;
    lite_cjson_t lite_item_pk, lite_item_time;
    lite_cjson_t lite_item_version, lite_item_configid, lite_item_configsize, lite_item_gettype, lite_item_sign,
                 lite_item_signmethod, lite_item_url, lite_item_data, lite_item_message;

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_ENOS_PROT_EVENT, "EnOS event type: %d", type);
    if (payload) {
        log_debug("enos", "receive payload %.*s", strlen(payload), payload);
    }

    if (payload) {
        res = dm_utils_json_parse(payload, strlen(payload), cJSON_Invalid, &lite);
        if (res != SUCCESS_RETURN) {
            return;
        }
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_ID, strlen(IOTX_ENOS_KEY_ID), cJSON_Invalid, &lite_item_id);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_DEVID, strlen(IOTX_ENOS_KEY_DEVID), cJSON_Invalid,
                                  &lite_item_devid);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_SERVICEID, strlen(IOTX_ENOS_KEY_SERVICEID), cJSON_Invalid,
                                  &lite_item_serviceid);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_PAYLOAD, strlen(IOTX_ENOS_KEY_PAYLOAD), cJSON_Invalid,
                                  &lite_item_payload);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_CTX, strlen(IOTX_ENOS_KEY_CTX), cJSON_Invalid, &lite_item_ctx);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_CODE, strlen(IOTX_ENOS_KEY_CODE), cJSON_Invalid, &lite_item_code);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_EVENTID, strlen(IOTX_ENOS_KEY_EVENTID), cJSON_Invalid,
                                  &lite_item_eventid);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_UTC, strlen(IOTX_ENOS_KEY_UTC), cJSON_Invalid, &lite_item_utc);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_TOPO, strlen(IOTX_ENOS_KEY_TOPO), cJSON_Invalid,
                                  &lite_item_topo);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_PRODUCT_KEY, strlen(IOTX_ENOS_KEY_PRODUCT_KEY), cJSON_Invalid,
                                  &lite_item_pk);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_TIME, strlen(IOTX_ENOS_KEY_TIME), cJSON_Invalid,
                                  &lite_item_time);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_VERSION, strlen(IOTX_ENOS_KEY_VERSION), cJSON_Invalid,
                                  &lite_item_version);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_CONFIG_ID, strlen(IOTX_ENOS_KEY_CONFIG_ID), cJSON_Invalid,
                                  &lite_item_configid);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_CONFIG_SIZE, strlen(IOTX_ENOS_KEY_CONFIG_SIZE), cJSON_Invalid,
                                  &lite_item_configsize);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_GET_TYPE, strlen(IOTX_ENOS_KEY_GET_TYPE), cJSON_Invalid,
                                  &lite_item_gettype);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_SIGN, strlen(IOTX_ENOS_KEY_SIGN), cJSON_Invalid,
                                  &lite_item_sign);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_SIGN_METHOD, strlen(IOTX_ENOS_KEY_SIGN_METHOD), cJSON_Invalid,
                                  &lite_item_signmethod);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_URL, strlen(IOTX_ENOS_KEY_URL), cJSON_Invalid,
                                  &lite_item_url);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_DATA, strlen(IOTX_ENOS_KEY_DATA), cJSON_Invalid,
                                  &lite_item_data);
        dm_utils_json_object_item(&lite, IOTX_ENOS_KEY_MESSAGE, strlen(IOTX_ENOS_KEY_MESSAGE), cJSON_Invalid,
                                  &lite_item_message);
    }

    switch (type) {
        case IOTX_DM_EVENT_CLOUD_CONNECTED: {
            callback = iotx_event_callback(ITE_CONNECT_SUCC);
            if (callback) {
                ((int (*)(void))callback)();
            }
        }
        break;
        case IOTX_DM_EVENT_CLOUD_DISCONNECT: {
            callback = iotx_event_callback(ITE_DISCONNECTED);
            if (callback) {
                ((int (*)(void))callback)();
            }
        }
        break;
        case IOTX_DM_EVENT_INITIALIZED: {
            if (payload == NULL || lite_item_devid.type != cJSON_Number) {
                return;
            }

            iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_ENOS_PROT_EVENT, "dev init, devid: %d",
                             lite_item_devid.value_int);

            callback = iotx_event_callback(ITE_INITIALIZE_COMPLETED);
            if (callback) {
                ((int (*)(const int))callback)(lite_item_devid.value_int);
            }
        }
        break;
        case IOTX_DM_EVENT_MODEL_DOWN_RAW: {
            int raw_data_len = 0;
            unsigned char *raw_data = NULL;

            if (payload == NULL || lite_item_devid.type != cJSON_Number || lite_item_payload.type != cJSON_String) {
                return;
            }

            iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_ENOS_PROT_EVENT, "down raw, devid: %d, raw: %.*s",
                             lite_item_devid.value_int,
                             lite_item_payload.value_length, lite_item_payload.value);

            raw_data_len = lite_item_payload.value_length / 2;
            raw_data = IMPL_ENOS_MALLOC(raw_data_len);
            if (raw_data == NULL) {
                iotx_state_event(ITE_STATE_DEV_MODEL, STATE_SYS_DEPEND_MALLOC, NULL);
                return;
            }
            LITE_hexstr_convert(lite_item_payload.value, lite_item_payload.value_length, raw_data, raw_data_len);

            HEXDUMP_DEBUG(raw_data, raw_data_len);
            callback = iotx_event_callback(ITE_RAWDATA_ARRIVED);
            if (callback) {
                ((int (*)(const int, const unsigned char *, const int))callback)(lite_item_devid.value_int, raw_data, raw_data_len);
            }

            IMPL_ENOS_FREE(raw_data);
        }
        break;
        case IOTX_DM_EVENT_MODEL_UP_RAW_REPLY: {
            int raw_data_len = 0;
            unsigned char *raw_data = NULL;

            if (payload == NULL || lite_item_devid.type != cJSON_Number || lite_item_payload.type != cJSON_String) {
                return;
            }

            iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_ENOS_PROT_EVENT, "up raw reply, devid: %d, raw: %.*s",
                             lite_item_devid.value_int,
                             lite_item_payload.value_length, lite_item_payload.value);

            raw_data_len = lite_item_payload.value_length / 2;
            raw_data = IMPL_ENOS_MALLOC(raw_data_len);
            if (raw_data == NULL) {
                iotx_state_event(ITE_STATE_DEV_MODEL, STATE_SYS_DEPEND_MALLOC, NULL);
                return;
            }
            memset(raw_data, 0, raw_data_len);
            LITE_hexstr_convert(lite_item_payload.value, lite_item_payload.value_length, raw_data, raw_data_len);

            HEXDUMP_DEBUG(raw_data, raw_data_len);

            callback = iotx_event_callback(ITE_RAWDATA_ARRIVED);
            if (callback) {
                ((int (*)(const int, const unsigned char *, const int))callback)(lite_item_devid.value_int, raw_data, raw_data_len);
            }

            IMPL_ENOS_FREE(raw_data);
        }
        break;
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
        case IOTX_DM_EVENT_THING_SERVICE_INVOKE: {
            int response_len = 0;
            char *request = NULL, *response = NULL;

            if (payload == NULL || lite_item_id.type != cJSON_String || lite_item_devid.type != cJSON_Number ||
                lite_item_serviceid.type != cJSON_String || lite_item_payload.type != cJSON_Object) {
                return;
            }

            iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_ENOS_PROT_EVENT,
                             "service req, msgid: %.*s, devid: %d, serviceid: %.*s, payload: %.*s",
                             lite_item_id.value_length, lite_item_id.value,
                             lite_item_devid.value_int,
                             lite_item_serviceid.value_length, lite_item_serviceid.value,
                             lite_item_payload.value_length, lite_item_payload.value);

            request = IMPL_ENOS_MALLOC(lite_item_payload.value_length + 1);
            if (request == NULL) {
                iotx_state_event(ITE_STATE_DEV_MODEL, STATE_SYS_DEPEND_MALLOC, NULL);
                return;
            }
            memset(request, 0, lite_item_payload.value_length + 1);
            memcpy(request, lite_item_payload.value, lite_item_payload.value_length);

            callback = iotx_event_callback(ITE_SERVICE_REQUEST);
            if (callback) {
                void *service_ctx = NULL;
                _enos_command_list_insert(IOTX_SERVICE_REQ_TYPE_GENERAL, lite_item_id.value, NULL, 0, NULL, &service_ctx);
                if (service_ctx != NULL) {
                    res = ((int (*)(int, const char *, int, const char *, int,
                                    void *))callback)(lite_item_devid.value_int,
                                                      lite_item_serviceid.value,
                                                      lite_item_serviceid.value_length,
                                                      /* lite_item_id.value, lite_item_id.value_length, */
                                                      request, lite_item_payload.value_length, service_ctx);
                }
            }

            IMPL_ENOS_FREE(request);
        }
        break;
        case IOTX_DM_EVENT_MEASUREPOINT_SET: {
            char *measurepoint_payload = NULL;

            if (payload == NULL || lite_item_devid.type != cJSON_Number || lite_item_payload.type != cJSON_Object) {
                return;
            }

            iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_ENOS_PROT_EVENT, "measurepoint set, devid: %d,, id: %.*s, payload: %.*s",
                             lite_item_devid.value_int,
                             lite_item_id.value_length, lite_item_id.value,
                             lite_item_payload.value_length, lite_item_payload.value);

            measurepoint_payload = IMPL_ENOS_MALLOC(lite_item_payload.value_length + 1);
            if (measurepoint_payload == NULL) {
                iotx_state_event(ITE_STATE_DEV_MODEL, STATE_SYS_DEPEND_MALLOC, NULL);
                return;
            }
            memset(measurepoint_payload, 0, lite_item_payload.value_length + 1);
            memcpy(measurepoint_payload, lite_item_payload.value, lite_item_payload.value_length);
            callback = iotx_event_callback(ITE_MEASUREPOINT_SET);
            if (callback) {
                void *measurepoint_set_ctx = NULL;
                _enos_command_list_insert(IOTX_SERVICE_REQ_TYPE_GENERAL, lite_item_id.value, NULL, 0, NULL, &measurepoint_set_ctx);
                ((int (*)(const int, const char*, const int, void*))callback)(lite_item_devid.value_int,
                        measurepoint_payload, lite_item_payload.value_length, measurepoint_set_ctx);
            }

            IMPL_ENOS_FREE(measurepoint_payload);
        }
        break;
        case IOTX_DM_EVENT_THING_REPLY: {
            char *user_eventid = NULL;
            char *user_payload = NULL;

            if (payload == NULL || lite_item_id.type != cJSON_Number || lite_item_code.type != cJSON_Number ||
                lite_item_devid.type != cJSON_Number || lite_item_eventid.type != cJSON_String
                || lite_item_payload.type != cJSON_String) {
                return;
            }

            iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_ENOS_PROT_EVENT, "receive reply, eventID: %.*s",
                             lite_item_eventid.value_length, lite_item_eventid.value);

            user_eventid = IMPL_ENOS_MALLOC(lite_item_eventid.value_length + 1);
            if (user_eventid == NULL) {
                iotx_state_event(ITE_STATE_DEV_MODEL, STATE_SYS_DEPEND_MALLOC, NULL);
                return;
            }
            memset(user_eventid, 0, lite_item_eventid.value_length + 1);
            memcpy(user_eventid, lite_item_eventid.value, lite_item_eventid.value_length);

            user_payload = IMPL_ENOS_MALLOC(lite_item_payload.value_length + 1);
            if (user_payload == NULL) {
                iotx_state_event(ITE_STATE_DEV_MODEL, STATE_SYS_DEPEND_MALLOC, NULL);
                IMPL_ENOS_FREE(user_eventid);
                return;
            }
            memset(user_payload, 0, lite_item_payload.value_length + 1);
            memcpy(user_payload, lite_item_payload.value, lite_item_payload.value_length);


            callback = iotx_event_callback(ITE_THING_REPLY);
            if (callback) {
                ((int (*)(const int, const int, const int, const char *, const int, const char *,
                          const int))callback)
                (lite_item_devid.value_int, lite_item_id.value_int, lite_item_code.value_int,
                 user_eventid, lite_item_eventid.value_length, user_payload, lite_item_payload.value_length);
            }

            IMPL_ENOS_FREE(user_eventid);
            IMPL_ENOS_FREE(user_payload);
        }
        break;
#endif
        case IOTX_DM_EVENT_CLOUD_ERROR: {
            char *err_data = NULL;
            char *err_detail = NULL;

            if (payload == NULL) {
                return;
            }
            if (payload == NULL || lite_item_code.type != cJSON_Number) {
                return;
            }

            err_data = IMPL_ENOS_MALLOC(lite_item_data.value_length + 1);
            if (err_data == NULL) {
                iotx_state_event(ITE_STATE_DEV_MODEL, STATE_SYS_DEPEND_MALLOC, NULL);
                return;
            }

            memset(err_data, 0, lite_item_data.value_length + 1);
            memcpy(err_data, lite_item_data.value, lite_item_data.value_length);

            err_detail = IMPL_ENOS_MALLOC(lite_item_message.value_length + 1);
            if (err_detail == NULL) {
                IMPL_ENOS_FREE(err_data);
                iotx_state_event(ITE_STATE_DEV_MODEL, STATE_SYS_DEPEND_MALLOC, NULL);
                return;
            }

            memset(err_detail, 0, lite_item_message.value_length + 1);
            memcpy(err_detail, lite_item_message.value, lite_item_message.value_length);

            callback = iotx_event_callback(ITE_CLOUD_ERROR);
            if (callback) {
                ((int (*)(int, const char *, const char *))callback)(lite_item_code.value_int, err_data, err_detail);
            }
            IMPL_ENOS_FREE(err_data);
            IMPL_ENOS_FREE(err_detail);
        }
        break;
#ifdef DEVICE_MODEL_GATEWAY
        case IOTX_DM_EVENT_TOPO_GET_REPLY: {
            char *topo_list = NULL;

            if (payload == NULL || lite_item_id.type != cJSON_Number || lite_item_devid.type != cJSON_Number ||
                lite_item_code.type != cJSON_Number || lite_item_topo.type != cJSON_Array) {
                return;
            }

            iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_ENOS_PROT_EVENT,
                             "topic get reply, msgid = %d, devid = %d, code = %d",
                             lite_item_id.value_int,
                             lite_item_devid.value_int,
                             lite_item_code.value_int);

            topo_list = IMPL_ENOS_MALLOC(lite_item_topo.value_length + 1);
            if (topo_list == NULL) {
                iotx_state_event(ITE_STATE_DEV_MODEL, STATE_SYS_DEPEND_MALLOC, NULL);
                return;
            }
            memset(topo_list, 0, lite_item_topo.value_length + 1);
            memcpy(topo_list, lite_item_topo.value, lite_item_topo.value_length);

            callback = iotx_event_callback(ITE_TOPOLIST_REPLY);
            if (callback) {
                ((int (*)(const int, const int, const int, const char *, const int))callback)(lite_item_devid.value_int,
                        lite_item_id.value_int,
                        lite_item_code.value_int, topo_list, lite_item_topo.value_length);
            }

            IMPL_ENOS_FREE(topo_list);
        }
        break;
        case IOTX_DM_EVENT_TOPO_DELETE_REPLY:
        case IOTX_DM_EVENT_TOPO_ADD_REPLY:
        case IOTX_DM_EVENT_SUBDEV_REGISTER_REPLY:
        case IOTX_DM_EVENT_PROXY_PRODUCT_REGISTER_REPLY:
        case IOTX_DM_EVENT_COMBINE_LOGIN_REPLY:
        case IOTX_DM_EVENT_COMBINE_LOGOUT_REPLY: {
            if (payload == NULL || lite_item_id.type != cJSON_Number || lite_item_devid.type != cJSON_Number ||
                lite_item_code.type != cJSON_Number) {
                return;
            }
            iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_ENOS_PROT_EVENT,
                             "login/logout reply, msgid = %d, devid = %d, code = %d",
                             lite_item_id.value_int,
                             lite_item_devid.value_int,
                             lite_item_code.value_int);

            _iotx_enos_upstream_mutex_lock();
            _iotx_enos_upstream_callback_remove(lite_item_id.value_int, lite_item_code.value_int);
            _iotx_enos_upstream_mutex_unlock();
        }
        break;
        case IOTX_DM_EVENT_GATEWAY_PERMIT: {
            char *product_key = "";

            if (payload == NULL || lite_item_time.type != cJSON_Number) {
                return;
            }

            if (lite_item_pk.type == cJSON_String) {
                product_key = IMPL_ENOS_MALLOC(lite_item_pk.value_length + 1);
                if (product_key == NULL) {
                    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_SYS_DEPEND_MALLOC, NULL);
                    return;
                }
                memset(product_key, 0, lite_item_pk.value_length + 1);
                memcpy(product_key, lite_item_pk.value, lite_item_pk.value_length);
            }

            iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_ENOS_PROT_EVENT, "permitJoin recv, pk = %.*s, time = %d",
                             lite_item_pk.value_length, lite_item_pk.value,
                             lite_item_time.value_int);

            callback = iotx_event_callback(ITE_PERMIT_JOIN);
            if (callback) {
                ((int (*)(const char *, int))callback)((const char *)product_key, (const int)lite_item_time.value_int);
            }

            if (lite_item_pk.type == cJSON_String) {
                IMPL_ENOS_FREE(product_key);
            }
        }
        break;
#endif
        default: {
        }
        break;
    }
}

static int _iotx_enos_master_open(iotx_enos_dev_meta_info_t *meta_info)
{
    int res = 0;
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();

    if (ctx->is_opened) {
        return STATE_DEV_MODEL_MASTER_ALREADY_OPEN;
    }
    ctx->is_opened = 1;

    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_KEY, meta_info->product_key);
    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_SECRET, meta_info->product_secret);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_KEY, meta_info->device_key);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_SECRET, meta_info->device_secret);

    /* Create Mutex */
    ctx->mutex = HAL_MutexCreate();
    if (ctx->mutex == NULL) {
        ctx->is_opened = 0;
        return STATE_SYS_DEPEND_MUTEX_CREATE;
    }

#ifdef DEVICE_MODEL_GATEWAY
    service_ctx->upstream_mutex = HAL_MutexCreate();
    if (service_ctx->upstream_mutex == NULL) {
        HAL_MutexDestroy(service_ctx->mutex);
        service_ctx->is_opened = 0;
        return STATE_SYS_DEPEND_MUTEX_CREATE;
    }
#endif

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
    ctx->command_list_mutex = HAL_MutexCreate();
    if (ctx->command_list_mutex == NULL) {
        HAL_MutexDestroy(ctx->mutex);
        HAL_MutexDestroy(ctx->upstream_mutex);
        ctx->is_opened = 0;
        return STATE_SYS_DEPEND_MUTEX_CREATE;
    }
    ctx->command_list_num = 0;
#endif /* #if !defined(DEVICE_MODEL_RAWDATA_SOLO) */

    res = iotx_dm_open();
    if (res != SUCCESS_RETURN) {
#ifdef DEVICE_MODEL_GATEWAY
        HAL_MutexDestroy(service_ctx->upstream_mutex);
#endif
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
        HAL_MutexDestroy(ctx->command_list_mutex);
#endif /* #if !defined(DEVICE_MODEL_RAWDATA_SOLO) */
        HAL_MutexDestroy(ctx->mutex);
        ctx->is_opened = 0;
        return res;
    }

    INIT_LIST_HEAD(&ctx->upstream_sync_callback_list);
    INIT_LIST_HEAD(&ctx->downstream_command_list);

    return SUCCESS_RETURN;
}

#ifdef DEVICE_MODEL_GATEWAY
static int _iotx_enos_slave_open(iotx_enos_dev_meta_info_t *meta_info)
{
    int res, devid;
    iotx_enos_ctx_t *service_ctx = _iotx_enos_get_ctx();

    if (!service_ctx->is_opened) {
        return STATE_DEV_MODEL_MASTER_NOT_OPEN_YET;
    }

    res = iotx_dm_subdev_create(meta_info->product_key, meta_info->product_secret, meta_info->device_key,
                                meta_info->device_secret, &devid);
    if (res < 0) {
        return res;
    } else {
        return devid;
    }
}
#endif

static int _iotx_enos_master_connect(void)
{
    int res = 0;
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();
    iotx_dm_init_params_t dm_init_params;
    iotx_dm_event_types_t type;

    if (ctx->is_connected) {
        return STATE_DEV_MODEL_MASTER_ALREADY_CONNECT;
    }
    ctx->is_connected = 1;

    memset(&dm_init_params, 0, sizeof(iotx_dm_init_params_t));
    dm_init_params.event_callback = _iotx_enos_event_callback;

    res = iotx_dm_connect(&dm_init_params);
    if (res != SUCCESS_RETURN) {
        ctx->is_connected = 0;
        return res;
    }

    res = iotx_dm_subscribe(IOTX_DM_LOCAL_NODE_DEVID);
    if (res != SUCCESS_RETURN) {
        ctx->is_connected = 0;
        return res;
    }

    type = IOTX_DM_EVENT_INITIALIZED;
    _iotx_enos_event_callback(type, "{\"devid\":0}");

    ctx->yield_running = 1;
    return SUCCESS_RETURN;
}

#ifdef DEVICE_MODEL_GATEWAY
static int _iotx_enos_slave_connect(int devid)
{
    int res = 0, msgid = 0, code = 0;
    int proxy_product_register = 0;
    iotx_enos_ctx_t *service_ctx = _iotx_enos_get_ctx();
    iotx_enos_upstream_sync_callback_node_t *node = NULL;
    void *semaphore = NULL;

    if (service_ctx->is_connected == 0) {
        return STATE_DEV_MODEL_MASTER_NOT_CONNECT_YET;
    }

    if (devid <= 0) {
        return STATE_USER_INPUT_DEVID;
    }

    res = iotx_dm_get_opt(DM_OPT_PROXY_PRODUCT_REGISTER, (void *)&proxy_product_register);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    /* Subdev Register */
    if (proxy_product_register) {
        res = iotx_dm_subdev_proxy_register(devid);
        if (res < SUCCESS_RETURN) {
            return res;
        }
    } else {
        res = iotx_dm_subdev_register(devid);
        if (res < SUCCESS_RETURN) {
            return res;
        }
    }

    if (res > SUCCESS_RETURN) {
        semaphore = HAL_SemaphoreCreate();
        if (semaphore == NULL) {
            return STATE_SYS_DEPEND_SEMAPHORE_CREATE;
        }

        msgid = res;

        _iotx_enos_upstream_mutex_lock();
        res = _iotx_enos_upstream_sync_callback_list_insert(msgid, semaphore, &node);
        if (res != SUCCESS_RETURN) {
            HAL_SemaphoreDestroy(semaphore);
            _iotx_enos_upstream_mutex_unlock();
            return res;
        }
        _iotx_enos_upstream_mutex_unlock();

        res = HAL_SemaphoreWait(semaphore, IOTX_ENOS_SYNC_DEFAULT_TIMEOUT_MS);
        if (res < SUCCESS_RETURN) {
            _iotx_enos_upstream_mutex_lock();
            _iotx_enos_upstream_sync_callback_list_remove(msgid);
            _iotx_enos_upstream_mutex_unlock();
            return STATE_SYS_DEPEND_SEMAPHORE_WAIT;
        }

        _iotx_enos_upstream_mutex_lock();
        code = node->code;
        _iotx_enos_upstream_sync_callback_list_remove(msgid);
        if (code != SUCCESS_RETURN) {
            _iotx_enos_upstream_mutex_unlock();
            iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_REFUSED_BY_CLOUD, "refuse register for devid: %d", devid);
            return STATE_DEV_MODEL_REFUSED_BY_CLOUD;
        }
        _iotx_enos_upstream_mutex_unlock();
    }

    /* Subdev Add Topo */
    res = iotx_dm_subdev_topo_add(devid);
    if (res < SUCCESS_RETURN) {
        _iotx_enos_mutex_unlock();
        return res;
    }
    semaphore = HAL_SemaphoreCreate();
    if (semaphore == NULL) {
        _iotx_enos_mutex_unlock();
        return STATE_SYS_DEPEND_SEMAPHORE_CREATE;
    }

    msgid = res;
    _iotx_enos_upstream_mutex_lock();
    res = _iotx_enos_upstream_sync_callback_list_insert(msgid, semaphore, &node);
    if (res != SUCCESS_RETURN) {
        HAL_SemaphoreDestroy(semaphore);
        _iotx_enos_upstream_mutex_unlock();
        return res;
    }
    _iotx_enos_upstream_mutex_unlock();

    res = HAL_SemaphoreWait(semaphore, IOTX_ENOS_SYNC_DEFAULT_TIMEOUT_MS);
    if (res < SUCCESS_RETURN) {
        _iotx_enos_upstream_mutex_lock();
        _iotx_enos_upstream_sync_callback_list_remove(msgid);
        _iotx_enos_upstream_mutex_unlock();
        return STATE_SYS_DEPEND_SEMAPHORE_WAIT;
    }

    _iotx_enos_upstream_mutex_lock();
    code = node->code;
    _iotx_enos_upstream_sync_callback_list_remove(msgid);
    if (code != SUCCESS_RETURN) {
        _iotx_enos_upstream_mutex_unlock();
        iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_REFUSED_BY_CLOUD, "refuse to add topo for devid: %d", devid);
        return STATE_DEV_MODEL_REFUSED_BY_CLOUD;
    }
    _iotx_enos_upstream_mutex_unlock();

    return SUCCESS_RETURN;
}

static int _iotx_enos_subdev_delete_topo(int devid)
{
    int res = 0, msgid = 0, code = 0;
    iotx_enos_ctx_t *service_ctx = _iotx_enos_get_ctx();
    iotx_enos_upstream_sync_callback_node_t *node = NULL;
    void *semaphore = NULL;

    if (service_ctx->is_connected == 0) {
        return STATE_DEV_MODEL_MASTER_NOT_CONNECT_YET;
    }

    if (devid <= 0) {
        return STATE_USER_INPUT_DEVID;
    }

    /* Subdev Delete Topo */
    res = iotx_dm_subdev_topo_del(devid);
    if (res < SUCCESS_RETURN) {
        return res;
    }
    msgid = res;

    semaphore = HAL_SemaphoreCreate();
    if (semaphore == NULL) {
        return STATE_SYS_DEPEND_SEMAPHORE_CREATE;
    }

    _iotx_enos_upstream_mutex_lock();
    res = _iotx_enos_upstream_sync_callback_list_insert(msgid, semaphore, &node);
    if (res != SUCCESS_RETURN) {
        HAL_SemaphoreDestroy(semaphore);
        _iotx_enos_upstream_mutex_unlock();
        return res;
    }
    _iotx_enos_upstream_mutex_unlock();

    res = HAL_SemaphoreWait(semaphore, IOTX_ENOS_SYNC_DEFAULT_TIMEOUT_MS);
    if (res < SUCCESS_RETURN) {
        _iotx_enos_upstream_mutex_lock();
        _iotx_enos_upstream_sync_callback_list_remove(msgid);
        _iotx_enos_upstream_mutex_unlock();
        return STATE_SYS_DEPEND_SEMAPHORE_WAIT;
    }

    _iotx_enos_upstream_mutex_lock();
    code = node->code;
    _iotx_enos_upstream_sync_callback_list_remove(msgid);
    if (code != SUCCESS_RETURN) {
        _iotx_enos_upstream_mutex_unlock();
        iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_REFUSED_BY_CLOUD, "refuse to del topo for devid: %d", devid);
        return STATE_DEV_MODEL_REFUSED_BY_CLOUD;
    }
    _iotx_enos_upstream_mutex_unlock();

    return SUCCESS_RETURN;
}
#endif

static int _iotx_enos_master_close(void)
{
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();

    ctx->yield_running = 0;
    if (ctx->is_yield_running) {
        return STATE_DEV_MODEL_YIELD_RUNNING;
    }
    _iotx_enos_mutex_lock();
    if (ctx->is_opened == 0) {
        _iotx_enos_mutex_unlock();
        return STATE_DEV_MODEL_MASTER_NOT_OPEN_YET;
    }
    ctx->is_opened = 0;

    iotx_dm_close();
#ifdef DEVICE_MODEL_GATEWAY
    _iotx_enos_upstream_sync_callback_list_destroy();
    HAL_MutexDestroy(service_ctx->upstream_mutex);
#endif
    _iotx_enos_mutex_unlock();
    HAL_MutexDestroy(ctx->mutex);
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
    _enos_service_list_destroy();
    HAL_MutexDestroy(ctx->command_list_mutex);
#endif
    memset(ctx, 0, sizeof(iotx_enos_ctx_t));

    return SUCCESS_RETURN;
}

#ifdef DEVICE_MODEL_GATEWAY
static int _iotx_enos_slave_close(int devid)
{
    iotx_enos_ctx_t *service_ctx = _iotx_enos_get_ctx();

    _iotx_enos_mutex_lock();
    if (service_ctx->is_opened == 0) {
        _iotx_enos_mutex_unlock();
        return STATE_DEV_MODEL_MASTER_NOT_OPEN_YET;
    }

    /* Release Subdev Resources */
    iotx_dm_subdev_destroy(devid);

    _iotx_enos_mutex_unlock();

    return SUCCESS_RETURN;
}
#endif

int IOT_EnOS_Open(iotx_enos_dev_type_t dev_type, iotx_enos_dev_meta_info_t *meta_info)
{
    int res = 0;

    if (meta_info == NULL) {
        return STATE_USER_INPUT_META_INFO;
    }

    switch (dev_type) {
        case IOTX_ENOS_DEV_TYPE_MASTER: {
            res = _iotx_enos_master_open(meta_info);
            if (res == SUCCESS_RETURN) {
                res = IOTX_DM_LOCAL_NODE_DEVID;
            }
        }
        break;
        case IOTX_ENOS_DEV_TYPE_SLAVE: {
#ifdef DEVICE_MODEL_GATEWAY
            res = _iotx_enos_slave_open(meta_info);
#else
            res = STATE_DEV_MODEL_GATEWAY_NOT_ENABLED;
#endif
        }
        break;
        default: {
            res = STATE_USER_INPUT_DEVICE_TYPE;
        }
        break;
    }

    return res;
}

int IOT_EnOS_Connect(int devid)
{
    int res = 0;
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();

    if (devid < 0) {
        return STATE_USER_INPUT_INVALID;
    }

    if (ctx->is_opened == 0) {
        return STATE_DEV_MODEL_MASTER_NOT_OPEN_YET;
    }

    _iotx_enos_mutex_lock();

    if (devid == IOTX_DM_LOCAL_NODE_DEVID) {
        res = _iotx_enos_master_connect();
    } else {
#ifdef DEVICE_MODEL_GATEWAY
        res = _iotx_enos_slave_connect(devid);
#else
        res = STATE_DEV_MODEL_GATEWAY_NOT_ENABLED;
#endif
    }
    _iotx_enos_mutex_unlock();

    return res;
}

int IOT_EnOS_Yield(int timeout_ms)
{
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();
    int res = 0;

    if (ctx->yield_running == 0) {
        HAL_SleepMs(timeout_ms);
        return STATE_DEV_MODEL_YIELD_STOPPED;
    }

    ctx->is_yield_running = 1;
    if (timeout_ms <= 0) {
        ctx->is_yield_running = 0;
        return STATE_USER_INPUT_INVALID;
    }

    if (ctx->is_opened == 0 || ctx->is_connected == 0) {
        ctx->is_yield_running = 0;
        return STATE_DEV_MODEL_MASTER_NOT_CONNECT_YET;
    }

    res = iotx_dm_yield(timeout_ms);
    iotx_dm_dispatch();

#ifdef DEV_BIND_ENABLED
    IOT_Bind_Yield();
#endif

#ifdef DEVICE_MODEL_GATEWAY
    HAL_SleepMs(timeout_ms);
#endif
    ctx->is_yield_running = 0;

    return res;
}

int IOT_EnOS_Close(int devid)
{
    int res = 0;

    if (devid < 0) {
        return STATE_USER_INPUT_DEVID;
    }

    if (devid == IOTX_DM_LOCAL_NODE_DEVID) {
        res = _iotx_enos_master_close();
    } else {
#ifdef DEVICE_MODEL_GATEWAY
        res = _iotx_enos_slave_close(devid);
#else
        res = STATE_DEV_MODEL_GATEWAY_NOT_ENABLED;
#endif
    }

    return res;
}

#ifdef DEVICE_MODEL_GATEWAY
static int _iotx_enos_subdev_login(int devid)
{
    int res = 0, msgid = 0, code = 0;
    iotx_enos_upstream_sync_callback_node_t *node = NULL;
    void *semaphore = NULL;
    void *callback = NULL;

    res = iotx_dm_subdev_login(devid);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    msgid = res;
    semaphore = HAL_SemaphoreCreate();
    if (semaphore == NULL) {
        return STATE_SYS_DEPEND_SEMAPHORE_CREATE;
    }

    _iotx_enos_upstream_mutex_lock();
    res = _iotx_enos_upstream_sync_callback_list_insert(msgid, semaphore, &node);
    if (res != SUCCESS_RETURN) {
        HAL_SemaphoreDestroy(semaphore);
        _iotx_enos_upstream_mutex_unlock();
        return res;
    }
    _iotx_enos_upstream_mutex_unlock();

    res = HAL_SemaphoreWait(semaphore, IOTX_ENOS_SYNC_DEFAULT_TIMEOUT_MS);
    if (res < SUCCESS_RETURN) {
        _iotx_enos_upstream_mutex_lock();
        _iotx_enos_upstream_sync_callback_list_remove(msgid);
        _iotx_enos_upstream_mutex_unlock();
        return res;
    }

    _iotx_enos_upstream_mutex_lock();
    code = node->code;
    _iotx_enos_upstream_sync_callback_list_remove(msgid);
    if (code != SUCCESS_RETURN) {
        _iotx_enos_upstream_mutex_unlock();
        iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_REFUSED_BY_CLOUD, "refuse to login for devid: %d", devid);
        return STATE_DEV_MODEL_REFUSED_BY_CLOUD;
    }
    _iotx_enos_upstream_mutex_unlock();

    res = iotx_dm_subscribe(devid);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    callback = iotx_event_callback(ITE_INITIALIZE_COMPLETED);
    if (callback) {
        ((int (*)(const int))callback)(devid);
    }

    return SUCCESS_RETURN;
}

static int _iotx_enos_subdev_logout(int devid)
{
    int res = 0, msgid = 0, code = 0;
    iotx_enos_upstream_sync_callback_node_t *node = NULL;
    void *semaphore = NULL;

    res = iotx_dm_subdev_logout(devid);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    msgid = res;
    semaphore = HAL_SemaphoreCreate();
    if (semaphore == NULL) {
        return STATE_SYS_DEPEND_SEMAPHORE_CREATE;
    }

    _iotx_enos_upstream_mutex_lock();
    res = _iotx_enos_upstream_sync_callback_list_insert(msgid, semaphore, &node);
    if (res != SUCCESS_RETURN) {
        HAL_SemaphoreDestroy(semaphore);
        _iotx_enos_upstream_mutex_unlock();
        return res;
    }
    _iotx_enos_upstream_mutex_unlock();

    res = HAL_SemaphoreWait(semaphore, IOTX_ENOS_SYNC_DEFAULT_TIMEOUT_MS);
    if (res < SUCCESS_RETURN) {
        _iotx_enos_upstream_mutex_lock();
        _iotx_enos_upstream_sync_callback_list_remove(msgid);
        _iotx_enos_upstream_mutex_unlock();
        return STATE_SYS_DEPEND_SEMAPHORE_WAIT;
    }

    _iotx_enos_upstream_mutex_lock();
    code = node->code;
    _iotx_enos_upstream_sync_callback_list_remove(msgid);
    if (code != SUCCESS_RETURN) {
        _iotx_enos_upstream_mutex_unlock();
        iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_REFUSED_BY_CLOUD, "refuse to logout for devid: %d", devid);
        return STATE_DEV_MODEL_REFUSED_BY_CLOUD;
    }
    _iotx_enos_upstream_mutex_unlock();

    return SUCCESS_RETURN;
}
#endif
int IOT_EnOS_Report(int devid, iotx_enos_msg_type_t msg_type, unsigned char *payload, int payload_len)
{
    int res = 0;
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();

    if (devid < 0) {
        return STATE_USER_INPUT_DEVID;
    }

    if (msg_type >= IOTX_ENOS_MSG_MAX) {
        return STATE_USER_INPUT_MSG_TYPE;
    }

    if (ctx->is_opened == 0 || ctx->is_connected == 0) {
        return STATE_DEV_MODEL_MASTER_NOT_CONNECT_YET;
    }

    _iotx_enos_mutex_lock();
    switch (msg_type) {
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
        case ITM_MSG_QUERY_TAG:
        case ITM_MSG_UPDATE_TAG:
        case ITM_MSG_DELETE_TAG:
        case ITM_MSG_QUERY_ATTRIBUTE:
        case ITM_MSG_UPDATE_ATTRIBUTE:
        case ITM_MSG_DELETE_ATTRIBUTE:    
#ifdef DEVICE_MEASUREPOINT_RESUME
        case ITM_MSG_RESUME_MEASUREPOINT:
        case ITM_MSG_RESUME_MEASUREPOINT_BATCH:
#endif
        case ITM_MSG_POST_MEASUREPOINT:
        case ITM_MSG_POST_MEASUREPOINT_BATCH: {
            char *method = NULL;
            if (payload == NULL || payload_len <= 0) {
                _iotx_enos_mutex_unlock();
                return STATE_USER_INPUT_INVALID;
            }
            method = iotx_enos_get_method(msg_type);
            res = iotx_dm_upstream_common(devid, (char*)payload, payload_len, method);
        }
        break;
#endif
        case ITM_MSG_POST_RAW_DATA: {
            if (payload == NULL || payload_len <= 0) {
                _iotx_enos_mutex_unlock();
                return STATE_USER_INPUT_INVALID;
            }
            res = iotx_dm_post_rawdata(devid, (char *)payload, payload_len);
        }
        break;
        case ITM_MSG_LOGIN: {
#ifdef DEVICE_MODEL_GATEWAY
            res = _iotx_enos_subdev_login(devid);
#else
            res = STATE_DEV_MODEL_GATEWAY_NOT_ENABLED;
#endif
        }
        break;
        case ITM_MSG_LOGOUT: {
#ifdef DEVICE_MODEL_GATEWAY
            res = _iotx_enos_subdev_logout(devid);
#else
            res = STATE_DEV_MODEL_GATEWAY_NOT_ENABLED;
#endif
        }
        break;
        case ITM_MSG_DELETE_TOPO: {
#ifdef DEVICE_MODEL_GATEWAY
            res = _iotx_enos_subdev_delete_topo(devid);
#else
            res = STATE_DEV_MODEL_GATEWAY_NOT_ENABLED;
#endif
        }
        break;
#ifdef DEVICE_MODEL_GATEWAY
#ifdef DEVICE_MODEL_SUBDEV_OTA
        case ITM_MSG_REPORT_SUBDEV_FIRMWARE_VERSION: {
            res = iotx_dm_send_firmware_version(devid, (const char *)payload);
        }
        break;
#endif
#endif
#ifdef DEVICE_MEASUREPOINT_RESUME
        case ITM_MSG_MEASUREPOINT_RESUME_DATA: {
            if (payload == NULL || payload_len <= 0) {
                _iotx_enos_mutex_unlock();
                return STATE_USER_INPUT_INVALID;
            }
            res = iotx_dm_resume_measurepoint(devid, (char *)payload, payload_len);
        }
        break;
#endif /* #ifdef DEVICE_MEASUREPOINT_RESUME */
        default: {
            res = STATE_USER_INPUT_MSG_TYPE;
        }
        break;
    }
    _iotx_enos_mutex_unlock();
    return res;
}                                   

int IOT_EnOS_Query(int devid, iotx_enos_msg_type_t msg_type, unsigned char *payload, int payload_len)
{
    int res = 0;
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();

    if (devid < 0 || msg_type >= IOTX_ENOS_MSG_MAX) {
        return STATE_USER_INPUT_INVALID;
    }

    if (ctx->is_opened == 0 || ctx->is_connected == 0) {
        return STATE_DEV_MODEL_MASTER_NOT_CONNECT_YET;
    }

    _iotx_enos_mutex_lock();
    switch (msg_type) {
        case ITM_MSG_QUERY_TOPOLIST: {
#ifdef DEVICE_MODEL_GATEWAY
            res = iotx_dm_query_topo_list();
#else
            res = STATE_DEV_MODEL_GATEWAY_NOT_ENABLED;
#endif
        }
        break;
        default: {
            res = STATE_USER_INPUT_MSG_TYPE;
        }
        break;
    }
    _iotx_enos_mutex_unlock();
    return res;
}

int IOT_EnOS_TriggerEvent(int devid, char *eventid, int eventid_len, char *payload, int payload_len)
{
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
    int res = 0;
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();

    if (devid < 0 || eventid == NULL || eventid_len <= 0 || payload == NULL || payload_len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    if (ctx->is_opened == 0 || ctx->is_connected == 0) {
        return STATE_DEV_MODEL_MASTER_NOT_CONNECT_YET;
    }

    _iotx_enos_mutex_lock();
    res = iotx_dm_post_event(devid, eventid, eventid_len, payload, payload_len);
    _iotx_enos_mutex_unlock();

    return res;
#else
    return STATE_DEV_MODEL_IN_RAWDATA_SOLO;
#endif
}

int IOT_EnOS_CommandReply(int devid, iotx_command_type_t command_type,
        char *serviceid, int serviceid_len, char *payload, int payload_len, void *p_command_ctx)
{
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
    int res = 0;
    iotx_enos_ctx_t *ctx = _iotx_enos_get_ctx();

    char msgid[32] = {0};
    iotx_command_ctx_node_t *command_ctx = (iotx_command_ctx_node_t *)p_command_ctx;

    if (devid < 0 ||
        (command_type == IOTX_COMMAND_SERVICE_INVOKE && (serviceid == NULL || serviceid_len == 0)) ||
        payload == NULL || payload_len == 0 || command_ctx == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    if (ctx->is_opened == 0 || ctx->is_connected == 0) {
        return STATE_DEV_MODEL_MASTER_NOT_CONNECT_YET;
    }

    /* check if command ctx exist */
    if (_enos_command_list_search(command_ctx) != SUCCESS_RETURN) {
        return STATE_DEV_MODEL_SERVICE_CTX_NOT_EXIST;
    }

    /* msgid int to string */
    /* infra_int2str(command_ctx->msgid, msgid); */
    memcpy(msgid, command_ctx->msgid, command_ctx->msgid_len);

    /* rrpc service response */
    if (command_ctx->type == IOTX_SERVICE_REQ_TYPE_GENERAL) { /* alcs/normal service response */
        void *alcs_ctx = command_ctx->service_meta.general.service_ctx;
        if (command_type == IOTX_COMMAND_MEASUREPOINT_SET)
        {
            res = iotx_dm_send_measurepoint_set_response(devid, msgid, strlen(msgid), IOTX_DM_ERR_CODE_SUCCESS,
                    payload, payload_len, alcs_ctx);
        }
        else
        {
            res = iotx_dm_send_service_invoke_response(devid, msgid, strlen(msgid), IOTX_DM_ERR_CODE_SUCCESS,
                    serviceid, serviceid_len, payload, payload_len, alcs_ctx);
        }
    } else {
        res = STATE_DEV_MODEL_INTERNAL_ERROR;
    }

    _enos_command_list_mutex_lock();
    _enos_command_list_delete(command_ctx);
    _enos_command_list_mutex_unlock();

    return res;
#else
    return STATE_DEV_MODEL_IN_RAWDATA_SOLO;
#endif
}

#ifdef DEVICE_MODEL_GATEWAY
int iot_enos_subdev_query_id(char product_key[IOTX_PRODUCT_KEY_LEN + 1], char device_key[IOTX_DEVICE_KEY_LEN + 1])
{
    int res = -1;
    iotx_enos_ctx_t *service_ctx = _iotx_enos_get_ctx();

    if (service_ctx->is_opened == 0) {
        return STATE_DEV_MODEL_MASTER_NOT_OPEN_YET;
    }

    iotx_dm_subdev_query(product_key, device_key, &res);
    return res;
}
#endif /* #ifdef DEVICE_MODEL_GATEWAY */

#endif
