/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */

#include "iotx_dm_internal.h"

static dm_msg_ctx_t g_dm_msg_ctx;

static dm_msg_ctx_t *_dm_msg_get_ctx(void)
{
    return &g_dm_msg_ctx;
}

int dm_msg_init(void)
{
    dm_msg_ctx_t *ctx = _dm_msg_get_ctx();
    memset(ctx, 0, sizeof(dm_msg_ctx_t));

    return SUCCESS_RETURN;
}

int dm_msg_deinit(void)
{
    dm_msg_ctx_t *ctx = _dm_msg_get_ctx();
    memset(ctx, 0, sizeof(dm_msg_ctx_t));

    return SUCCESS_RETURN;
}

int _dm_msg_send_to_user(iotx_dm_event_types_t type, char *message)
{
    int res = 0;
    dm_ipc_msg_t *dipc_msg = NULL;

    dipc_msg = DM_malloc(sizeof(dm_ipc_msg_t));
    if (dipc_msg == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(dipc_msg, 0, sizeof(dm_ipc_msg_t));

    dipc_msg->type = type;
    dipc_msg->data = message;

    res = dm_ipc_msg_insert((void *)dipc_msg);
    if (res != SUCCESS_RETURN) {
        DM_free(dipc_msg);
        return res;
    }
    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_MSGQ_OPERATION, "msg enqueue w/ message type: %d", type);
    return SUCCESS_RETURN;
}

const char DM_MSG_SEND_MSG_TIMEOUT_FMT[] DM_READ_ONLY = "{\"id\":%d,\"code\":%d,\"devid\":%d}";
int dm_msg_send_msg_timeout_to_user(int msg_id, int devid, iotx_dm_event_types_t type)
{
    int res = 0, message_len = 0;
    char *message = NULL;

    message_len = strlen(DM_MSG_SEND_MSG_TIMEOUT_FMT) + DM_UTILS_UINT32_STRLEN * 3 + 1;
    message = DM_malloc(message_len + 1);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_SEND_MSG_TIMEOUT_FMT, msg_id, IOTX_DM_ERR_CODE_TIMEOUT, devid);

    res = _dm_msg_send_to_user(type, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
    }

    return res;
}
extern void *g_user_topic_callback;
int dm_msg_thing_model_user_sub(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN],
                                _IN_ char device_key[IOTX_DEVICE_KEY_LEN],
                                _IN_ char *payload, _IN_ int payload_len)
{
    int res = 0, devid = 0, message_len = 0;
    char *hexstr = NULL, *message = NULL;
    unsigned char *request = NULL;
    void *callback;

    if (product_key == NULL || device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN) ||
        payload == NULL || payload_len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    request = DM_malloc(payload_len + 1);
    if (request == NULL) {
        iotx_state_event(ITE_STATE_DEV_MODEL, STATE_SYS_DEPEND_MALLOC, NULL);
        return STATE_SYS_DEPEND_MALLOC;
    }

    memset(request, 0, payload_len + 1);
    memcpy(request, payload, payload_len);
    if (g_user_topic_callback) {
        ((int (*)(const int, const unsigned char *, const int))g_user_topic_callback)(devid, request, payload_len);
    }
    DM_free(request);

    return SUCCESS_RETURN;
}

int dm_msg_uri_parse_pkdk(_IN_ char *uri, _IN_ int uri_len, _IN_ int start_deli, _IN_ int end_deli,
                          _OU_ char product_key[IOTX_PRODUCT_KEY_LEN + 1], _OU_ char device_key[IOTX_DEVICE_KEY_LEN + 1])
{
    int res = 0, start = 0, end = 0, slice = 0;

    if (uri == NULL || uri_len <= 0 || product_key == NULL || device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) || (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    res = dm_utils_memtok(uri, uri_len, DM_URI_SERVICE_DELIMITER, start_deli, &start);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    res = dm_utils_memtok(uri, uri_len, DM_URI_SERVICE_DELIMITER, start_deli + 1, &slice);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    res = dm_utils_memtok(uri, uri_len, DM_URI_SERVICE_DELIMITER, end_deli, &end);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    memcpy(product_key, uri + start + 1, slice - start - 1);
    memcpy(device_key, uri + slice + 1, end - slice - 1);

    return SUCCESS_RETURN;
}

int dm_msg_request_parse(_IN_ char *payload, _IN_ int payload_len, _OU_ dm_msg_request_payload_t *request)
{
    lite_cjson_t lite;

    if (payload == NULL || payload_len <= 0 || request == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    if (dm_utils_json_parse(payload, payload_len, cJSON_Object, &lite) != SUCCESS_RETURN ||
        dm_utils_json_object_item(&lite, DM_MSG_KEY_ID, strlen(DM_MSG_KEY_ID), cJSON_String, &request->id) != SUCCESS_RETURN ||
        dm_utils_json_object_item(&lite, DM_MSG_KEY_VERSION, strlen(DM_MSG_KEY_VERSION), cJSON_String,
                                  &request->version) != SUCCESS_RETURN ||
        dm_utils_json_object_item(&lite, DM_MSG_KEY_METHOD, strlen(DM_MSG_KEY_METHOD), cJSON_String,
                                  &request->method) != SUCCESS_RETURN ||
        dm_utils_json_object_item(&lite, DM_MSG_KEY_PARAMS, strlen(DM_MSG_KEY_PARAMS), cJSON_Invalid,
                                  &request->params) != SUCCESS_RETURN) {
        return STATE_DEV_MODEL_ENOS_MSG_PARSE_FAILED;
    }

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE,
                     "cloud request msgid: %.*s, method: %.*s, params: %.*s",
                     request->id.value_length, request->id.value,
                     request->method.value_length, request->method.value,
                     request->params.value_length, request->params.value);

    return SUCCESS_RETURN;
}

int dm_msg_response_parse(_IN_ char *payload, _IN_ int payload_len, _OU_ dm_msg_response_payload_t *response)
{
    lite_cjson_t lite, lite_message;

    if (payload == NULL || payload_len <= 0 || response == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    if (dm_utils_json_parse(payload, payload_len, cJSON_Object, &lite) != SUCCESS_RETURN ||
        dm_utils_json_object_item(&lite, DM_MSG_KEY_ID, strlen(DM_MSG_KEY_ID), cJSON_String, &response->id) != SUCCESS_RETURN ||
        dm_utils_json_object_item(&lite, DM_MSG_KEY_CODE, strlen(DM_MSG_KEY_CODE), cJSON_Number,
                                  &response->code) != SUCCESS_RETURN ||
        dm_utils_json_object_item(&lite, DM_MSG_KEY_DATA, strlen(DM_MSG_KEY_DATA), cJSON_Invalid,
                                  &response->data) != SUCCESS_RETURN) {
        return STATE_DEV_MODEL_ENOS_MSG_PARSE_FAILED;
    }

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE,
                     "cloud response msgid: %.*s, code: %d, data: %.*s",
                     response->id.value_length, response->id.value,
                     response->code.value_int,
                     response->data.value_length, response->data.value);

    memset(&lite_message, 0, sizeof(lite_cjson_t));
    dm_utils_json_object_item(&lite, DM_MSG_KEY_MESSAGE, strlen(DM_MSG_KEY_MESSAGE), cJSON_Invalid,
                              &response->message);

    return SUCCESS_RETURN;
}

const char DM_MSG_REQUEST[] DM_READ_ONLY = "{\"id\":\"%d\",\"version\":\"%s\",\"params\":%.*s,\"method\":\"%s\"}";
int dm_msg_request(dm_msg_dest_type_t type, _IN_ dm_msg_request_t *request)
{
    int res = 0, payload_len = 0;
    char *payload = NULL, *uri = NULL;
    lite_cjson_t lite;

    if (request == NULL || request->params == NULL || request->method == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    /* Request URI */
    res = dm_utils_service_name(request->service_prefix, request->service_name,
                                request->product_key, request->device_key, &uri);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    payload_len = strlen(DM_MSG_REQUEST) + 10 + strlen(DM_MSG_VERSION) + request->params_len + strlen(
                              request->method) + 1;
    payload = DM_malloc(payload_len);
    if (payload == NULL) {
        DM_free(uri);
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(payload, 0, payload_len);
    HAL_Snprintf(payload, payload_len, DM_MSG_REQUEST, request->msgid,
                 DM_MSG_VERSION, request->params_len, request->params, request->method);

    memset(&lite, 0, sizeof(lite_cjson_t));
    res = lite_cjson_parse(payload, payload_len, &lite);
    if (res < SUCCESS_RETURN) {
        iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_WRONG_JSON_FORMAT, payload);
        DM_free(uri);
        DM_free(payload);
        return STATE_DEV_MODEL_WRONG_JSON_FORMAT;
    }

    if (type & DM_MSG_DEST_CLOUD) {
        res = dm_client_publish(uri, (unsigned char *)payload, strlen(payload), request->callback);
    }

    DM_free(uri);
    DM_free(payload);
    return res;
}

const char DM_MSG_RESPONSE_WITH_DATA[] DM_READ_ONLY = "{\"id\":\"%.*s\",\"code\":%d,\"data\":%.*s}";
int dm_msg_response(dm_msg_dest_type_t type, _IN_ dm_msg_request_payload_t *request, _IN_ dm_msg_response_t *response,
                    _IN_ char *data, _IN_ int data_len, _IN_ void *user_data)
{
    int res = 0, payload_len = 0;
    char *uri = NULL, *payload = NULL;
    lite_cjson_t lite;

    if (request == NULL || response == NULL || data == NULL || data_len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    /* Response URI */
    res = dm_utils_service_name(response->reply_prefix, response->reply_name,
                                response->product_key, response->device_key, &uri);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    /* Response Payload */
    payload_len = strlen(DM_MSG_RESPONSE_WITH_DATA) + request->id.value_length + DM_UTILS_UINT32_STRLEN + data_len + 1;
    payload = DM_malloc(payload_len);
    if (payload == NULL) {
        DM_free(uri);
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(payload, 0, payload_len);
    HAL_Snprintf(payload, payload_len, DM_MSG_RESPONSE_WITH_DATA,
                 request->id.value_length, request->id.value, response->code, data_len, data);

    memset(&lite, 0, sizeof(lite_cjson_t));
    res = lite_cjson_parse(payload, payload_len, &lite);
    if (res < SUCCESS_RETURN) {
        iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_WRONG_JSON_FORMAT, "wrong JSON format, uri: %s, payload: %s", uri,
                         payload);
        DM_free(uri);
        DM_free(payload);
        return FAIL_RETURN;
    }

    if (type & DM_MSG_DEST_CLOUD) {
        dm_client_publish(uri, (unsigned char *)payload, strlen(payload), NULL);
    }

    DM_free(uri);
    DM_free(payload);

    return SUCCESS_RETURN;
}


const char DM_MSG_THING_MODEL_DOWN_FMT[] DM_READ_ONLY = "{\"devid\":%d,\"payload\":\"%.*s\"}";
int dm_msg_thing_model_down_raw(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                                _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                                _IN_ char *payload, _IN_ int payload_len)
{
    int res = 0, devid = 0, message_len = 0;
    char *hexstr = NULL, *message = NULL;

    if (product_key == NULL || device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1) ||
        payload == NULL || payload_len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    res = dm_utils_hex_to_str((unsigned char *)payload, payload_len, &hexstr);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    message_len = strlen(DM_MSG_THING_MODEL_DOWN_FMT) + DM_UTILS_UINT32_STRLEN + strlen(hexstr) + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        DM_free(hexstr);
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_THING_MODEL_DOWN_FMT, devid, strlen(hexstr), hexstr);
    DM_free(hexstr);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_MODEL_DOWN_RAW, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}


const char DM_MSG_THING_MODEL_UP_RAW_REPLY_FMT[] DM_READ_ONLY = "{\"devid\":%d,\"payload\":\"%.*s\"}";
int dm_msg_thing_model_up_raw_reply(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                                    _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1], char *payload, int payload_len)
{
    int res = 0, devid = 0, message_len = 0;
    char *hexstr = NULL, *message = NULL;

    if (product_key == NULL || device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1) ||
        payload == NULL || payload_len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    res = dm_utils_hex_to_str((unsigned char *)payload, payload_len, &hexstr);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    message_len = strlen(DM_MSG_THING_MODEL_DOWN_FMT) + DM_UTILS_UINT32_STRLEN + strlen(hexstr) + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        DM_free(hexstr);
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_THING_MODEL_DOWN_FMT, devid, strlen(hexstr), hexstr);
    DM_free(hexstr);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_MODEL_UP_RAW_REPLY, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
const char DM_MSG_MEASUREPOINT_SET_FMT[] DM_READ_ONLY = "{\"devid\":%d,\"id\":%.*s,\"payload\":%.*s}";
int dm_msg_measurepoint_set(int devid, dm_msg_request_payload_t *request)
{
    int res = 0, message_len = 0;
    char *message = NULL;

    message_len = strlen(DM_MSG_MEASUREPOINT_SET_FMT) + DM_UTILS_UINT32_STRLEN + request->params.value_length + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_MEASUREPOINT_SET_FMT, devid, request->id.value_length, request->id.value,
                 request->params.value_length, request->params.value);
    res = _dm_msg_send_to_user(IOTX_DM_EVENT_MEASUREPOINT_SET, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }
    return SUCCESS_RETURN;
}

const char DM_MSG_SERVICE_INVOKE_FMT[] DM_READ_ONLY =
            "{\"id\":\"%.*s\",\"devid\":%d,\"serviceid\":\"%.*s\",\"payload\":%.*s,\"ctx\":\"%s\"}";
int dm_msg_thing_service_invoke(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                                 _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                                 char *identifier, int identifier_len, dm_msg_request_payload_t *request,  _IN_ void *ctx)
{
    int res = 0, devid = 0, message_len = 0;
    char *message = NULL;
    uintptr_t ctx_addr_num = (uintptr_t)ctx;
    char *ctx_addr_str = NULL;

    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    ctx_addr_str = DM_malloc(sizeof(uintptr_t) * 2 + 1);
    if (ctx_addr_str == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(ctx_addr_str, 0, sizeof(uintptr_t) * 2 + 1);
    infra_hex2str((unsigned char *)&ctx_addr_num, sizeof(uintptr_t), ctx_addr_str);

    message_len = strlen(DM_MSG_SERVICE_INVOKE_FMT) + request->id.value_length + DM_UTILS_UINT32_STRLEN + identifier_len +
                  request->params.value_length + strlen(ctx_addr_str)  + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        DM_free(ctx_addr_str);
        return STATE_SYS_DEPEND_MALLOC;
    }

    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_SERVICE_INVOKE_FMT, request->id.value_length, request->id.value, devid,
                 identifier_len, identifier,
                 request->params.value_length, request->params.value, ctx_addr_str);

    DM_free(ctx_addr_str);

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, "serviceID: %.*s", identifier_len, identifier);
    res = _dm_msg_send_to_user(IOTX_DM_EVENT_THING_SERVICE_INVOKE, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_THING_REPLY_FMT[] DM_READ_ONLY =
            "{\"id\":%d,\"code\":%d,\"devid\":%d,\"eventid\":\"%.*s\",\"payload\":\"%.*s\",\"message\":\"%.*s\"}";
int dm_msg_thing_reply(_IN_ char *identifier, _IN_ int identifier_len,
                                  _IN_ dm_msg_response_payload_t *response)
{
    int res = 0, devid = 0, id = 0, message_len = 0;
    char *message = NULL;
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_node_t *node = NULL;
#endif
    /* Message ID */
    if (response->id.value_length > DM_UTILS_UINT32_STRLEN) {
        return FAIL_RETURN;
    }
    memcpy(int_id, response->id.value, response->id.value_length);
    id = atoi(int_id);

#if !defined(DM_MESSAGE_CACHE_DISABLED)
    res = dm_msg_cache_search(id, &node);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }
    devid = node->devid;
#endif

    message_len = strlen(DM_MSG_THING_REPLY_FMT) + DM_UTILS_UINT32_STRLEN * 3 + strlen(
                              identifier) + response->message.value_length + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_THING_REPLY_FMT, id, response->code.value_int, devid,
                 identifier_len, identifier, 
                 response->data.value_length, response->data.value,
                 response->message.value_length, response->message.value);

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, "eventID: %.*s", identifier_len, identifier);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_THING_REPLY, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

#endif

#ifdef DEVICE_MODEL_GATEWAY
const char DM_MSG_TOPO_ADD_NOTIFY_USER_PAYLOAD[] DM_READ_ONLY =
            "{\"result\":%d,\"devid\":%d,\"product_key\":\"%s\",\"device_key\":\"%s\"}";
int dm_msg_topo_add_notify(_IN_ char *payload, _IN_ int payload_len)
{
    int ret = SUCCESS_RETURN, res = 0, index = 0, devid = 0, message_len = 0;
    lite_cjson_t lite, lite_item, lite_item_pk, lite_item_dk;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};
    char *message = NULL;

    memset(&lite, 0, sizeof(lite_cjson_t));
    res = lite_cjson_parse(payload, payload_len, &lite);
    if (res != SUCCESS_RETURN || !lite_cjson_is_array(&lite)) {
        return DM_JSON_PARSE_FAILED;
    }

    for (index = 0; index < lite.size; index++) {
        devid = 0;
        message_len = 0;
        message = NULL;
        memset(&lite_item, 0, sizeof(lite_cjson_t));
        memset(&lite_item_pk, 0, sizeof(lite_cjson_t));
        memset(&lite_item_dk, 0, sizeof(lite_cjson_t));
        memset(product_key, 0, IOTX_PRODUCT_KEY_LEN + 1);
        memset(device_key, 0, IOTX_DEVICE_KEY_LEN + 1);

        res = lite_cjson_array_item(&lite, index, &lite_item);
        if (res != SUCCESS_RETURN) {
            ret = FAIL_RETURN;
            continue;
        }

        res = lite_cjson_object_item(&lite_item, DM_MSG_KEY_PRODUCT_KEY, strlen(DM_MSG_KEY_PRODUCT_KEY), &lite_item_pk);
        if (res != SUCCESS_RETURN) {
            ret = FAIL_RETURN;
            continue;
        }

        res = lite_cjson_object_item(&lite_item, DM_MSG_KEY_DEVICE_KEY, strlen(DM_MSG_KEY_DEVICE_KEY), &lite_item_dk);
        if (res != SUCCESS_RETURN) {
            ret = FAIL_RETURN;
            continue;
        }

        if (lite_item_pk.value_length >= IOTX_PRODUCT_KEY_LEN + 1 ||
            lite_item_dk.value_length >= IOTX_DEVICE_KEY_LEN + 1) {
            ret = FAIL_RETURN;
            continue;
        }
        memcpy(product_key, lite_item_pk.value, lite_item_pk.value_length);
        memcpy(device_key, lite_item_dk.value, lite_item_dk.value_length);

        res = dm_mgr_device_create(IOTX_DM_DEVICE_SUBDEV, product_key, "", device_key, NULL, &devid);
        if (res != SUCCESS_RETURN) {
            ret = FAIL_RETURN;
        }

        /* Send To User */
        message_len = strlen(DM_MSG_TOPO_ADD_NOTIFY_USER_PAYLOAD) + 20 +
                      strlen(product_key) + strlen(device_key) + 1;
        message = DM_malloc(message_len);
        if (message == NULL) {
            ret = STATE_SYS_DEPEND_MALLOC;
            continue;
        }
        memset(message, 0, message_len);
        HAL_Snprintf(message, message_len, DM_MSG_TOPO_ADD_NOTIFY_USER_PAYLOAD, res, devid, product_key, device_key);
        res = _dm_msg_send_to_user(IOTX_DM_EVENT_TOPO_ADD_NOTIFY, message);
        if (res != SUCCESS_RETURN) {
            ret = FAIL_RETURN;
            DM_free(message);
        }

    }

    return ret;
}

const char DM_MSG_EVENT_THING_DISABLE_FMT[] DM_READ_ONLY = "{\"devid\":%d}";
int dm_msg_thing_disable(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                         _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1])
{
    int res = 0, devid = 0, message_len = 0;
    char *message = NULL;

    if (product_key == NULL || device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    res = dm_mgr_set_dev_disable(devid);

    message_len = strlen(DM_MSG_EVENT_THING_DISABLE_FMT) + DM_UTILS_UINT32_STRLEN + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_EVENT_THING_DISABLE_FMT, devid);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_THING_DISABLE, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_EVENT_THING_ENABLE_FMT[] DM_READ_ONLY = "{\"devid\":%d}";
int dm_msg_thing_enable(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                        _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1])
{
    int res = 0, devid = 0, message_len = 0;
    char *message = NULL;

    if (product_key == NULL || device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    res = dm_mgr_set_dev_enable(devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    message_len = strlen(DM_MSG_EVENT_THING_ENABLE_FMT) + DM_UTILS_UINT32_STRLEN + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_EVENT_THING_ENABLE_FMT, devid);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_THING_ENABLE, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_EVENT_THING_DELETE_FMT[] DM_READ_ONLY =
            "{\"res\":%d,\"productKey\":\"%s\",\"deviceKey\":\"%s\",\"devid\":%d}";
int dm_msg_thing_delete(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                        _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1])
{
    int res = 0, message_len = 0, devid = 0;
    char *message = NULL;

    if (product_key == NULL || device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    res = dm_mgr_device_destroy(devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    message_len = strlen(DM_MSG_EVENT_THING_DELETE_FMT) + strlen(product_key) + strlen(device_key) + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_EVENT_THING_DELETE_FMT, res, product_key, device_key, devid);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_THING_DELETE, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_EVENT_COMBINE_DISABLE_FMT[] DM_READ_ONLY = "{\"devid\":%d}";
int dm_msg_combine_disable(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                         _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1])
{
    int res = 0, devid = 0, message_len = 0;
    char *message = NULL;

    if (product_key == NULL || device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    res = dm_mgr_set_dev_disable(devid);

    message_len = strlen(DM_MSG_EVENT_COMBINE_DISABLE_FMT) + DM_UTILS_UINT32_STRLEN + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_EVENT_COMBINE_DISABLE_FMT, devid);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_COMBINE_DISABLE, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_EVENT_COMBINE_ENABLE_FMT[] DM_READ_ONLY = "{\"devid\":%d}";
int dm_msg_combine_enable(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                        _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1])
{
    int res = 0, devid = 0, message_len = 0;
    char *message = NULL;

    if (product_key == NULL || device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    res = dm_mgr_set_dev_enable(devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    message_len = strlen(DM_MSG_EVENT_COMBINE_ENABLE_FMT) + DM_UTILS_UINT32_STRLEN + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_EVENT_COMBINE_ENABLE_FMT, devid);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_COMBINE_ENABLE, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_EVENT_COMBINE_DELETE_FMT[] DM_READ_ONLY =
            "{\"res\":%d,\"productKey\":\"%s\",\"deviceKey\":\"%s\",\"devid\":%d}";
int dm_msg_combine_delete(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                        _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1])
{
    int res = 0, message_len = 0, devid = 0;
    char *message = NULL;

    if (product_key == NULL || device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    res = dm_mgr_device_destroy(devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    message_len = strlen(DM_MSG_EVENT_COMBINE_DELETE_FMT) + strlen(product_key) + strlen(device_key) + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_EVENT_COMBINE_DELETE_FMT, res, product_key, device_key, devid);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_COMBINE_DELETE, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

int dm_msg_thing_gateway_permit(_IN_ char *payload, _IN_ int payload_len)
{
    int res = 0, message_len = 0;
    char *message = NULL;
    lite_cjson_t lite;

    if (payload == NULL || payload_len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = lite_cjson_parse(payload, payload_len, &lite);
    if (res != SUCCESS_RETURN || !lite_cjson_is_object(&lite)) {
        return DM_JSON_PARSE_FAILED;
    }

    message_len = payload_len + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memcpy(message, payload, payload_len);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_GATEWAY_PERMIT, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_EVENT_SUBDEV_REGISTER_REPLY_FMT[] DM_READ_ONLY = "{\"id\":%d,\"code\":%d,\"devid\":%d}";
int dm_msg_thing_device_register_reply(dm_msg_response_payload_t *response)
{
    int res = 0, index = 0, message_len = 0, devid = 0;
    lite_cjson_t lite, lite_item, lite_item_pk, lite_item_dk, lite_item_ds;
    char *message = NULL;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};
    char device_secret[IOTX_DEVICE_SECRET_LEN + 1] = {0};
    char temp_id[DM_UTILS_UINT32_STRLEN] = {0};

    if (response == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    if (response->code.value_int != IOTX_DM_ERR_CODE_SUCCESS) {
        /* Send Message To User */
        memcpy(temp_id, response->id.value, response->id.value_length);
        message_len = strlen(DM_MSG_EVENT_SUBDEV_REGISTER_REPLY_FMT) + DM_UTILS_UINT32_STRLEN * 2 + 1;
        message = DM_malloc(message_len);
        if (message == NULL) {
            return FAIL_RETURN;
        }
        memset(message, 0, message_len);
        HAL_Snprintf(message, message_len, DM_MSG_EVENT_SUBDEV_REGISTER_REPLY_FMT, atoi(temp_id), response->code.value_int, 0);

        res = _dm_msg_send_to_user(IOTX_DM_EVENT_SUBDEV_REGISTER_REPLY, message);
        if (res != SUCCESS_RETURN) {
            DM_free(message);
        }
        return res;
    }

    res = lite_cjson_parse(response->data.value, response->data.value_length, &lite);
    if (res != SUCCESS_RETURN || !lite_cjson_is_array(&lite)) {
        return DM_JSON_PARSE_FAILED;
    }

    for (index = 0; index < lite.size; index++) {
        devid = 0;
        message_len = 0;
        message = NULL;
        memset(temp_id, 0, DM_UTILS_UINT32_STRLEN);
        memset(product_key, 0, IOTX_PRODUCT_KEY_LEN + 1);
        memset(device_key, 0, IOTX_DEVICE_KEY_LEN + 1);
        memset(&lite_item, 0, sizeof(lite_cjson_t));
        memset(&lite_item_pk, 0, sizeof(lite_cjson_t));
        memset(&lite_item_dk, 0, sizeof(lite_cjson_t));
        memset(&lite_item_ds, 0, sizeof(lite_cjson_t));

        /* Item */
        res = lite_cjson_array_item(&lite, index, &lite_item);
        if (res != SUCCESS_RETURN || !lite_cjson_is_object(&lite_item)) {
            continue;
        }

        /* Product Key */
        res = lite_cjson_object_item(&lite_item, DM_MSG_KEY_PRODUCT_KEY, strlen(DM_MSG_KEY_PRODUCT_KEY), &lite_item_pk);
        if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_pk)) {
            continue;
        }

        /* Device Key */
        res = lite_cjson_object_item(&lite_item, DM_MSG_KEY_DEVICE_KEY, strlen(DM_MSG_KEY_DEVICE_KEY), &lite_item_dk);
        if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_dk)) {
            continue;
        }

        /* Device Secret */
        res = lite_cjson_object_item(&lite_item, DM_MSG_KEY_DEVICE_SECRET, strlen(DM_MSG_KEY_DEVICE_SECRET), &lite_item_ds);
        if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_ds)) {
            continue;
        }

        /* Get Device ID */
        memcpy(product_key, lite_item_pk.value, lite_item_pk.value_length);
        memcpy(device_key, lite_item_dk.value, lite_item_dk.value_length);
        memcpy(device_secret, lite_item_ds.value, lite_item_ds.value_length);
        /* res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid); */
        res = dm_mgr_device_create(IOTX_DM_DEVICE_SUBDEV, product_key, "", device_key, device_secret, &devid); 
        if (res != SUCCESS_RETURN) {
            continue;
        }

        /* Update State Machine */
        if (response->code.value_int == IOTX_DM_ERR_CODE_SUCCESS) {
            dm_mgr_set_dev_status(devid, IOTX_DM_DEV_STATUS_REGISTERED);
        }

        /* Set Device Secret */
        res = dm_mgr_set_device_secret(devid, device_secret);
        if (res != SUCCESS_RETURN) {
            continue;
        }

        /* Send Message To User */
        memcpy(temp_id, response->id.value, response->id.value_length);
        message_len = strlen(DM_MSG_EVENT_SUBDEV_REGISTER_REPLY_FMT) + DM_UTILS_UINT32_STRLEN * 2 + 1;
        message = DM_malloc(message_len);
        if (message == NULL) {
            continue;
        }
        memset(message, 0, message_len);
        HAL_Snprintf(message, message_len, DM_MSG_EVENT_SUBDEV_REGISTER_REPLY_FMT, atoi(temp_id), response->code.value_int,
                     devid);

        res = _dm_msg_send_to_user(IOTX_DM_EVENT_SUBDEV_REGISTER_REPLY, message);
        if (res != SUCCESS_RETURN) {
            DM_free(message);
        }
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_EVENT_SUBDEV_UNREGISTER_REPLY_FMT[] DM_READ_ONLY = "{\"id\":%d,\"code\":%d,\"devid\":%d}";
int dm_msg_thing_sub_unregister_reply(dm_msg_response_payload_t *response)
{
    int res = 0, devid = 0, id, message_len = 0;
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};
    char *message = NULL;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_node_t *node = NULL;
#endif

    if (response == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    if (response->id.value_length > DM_UTILS_UINT32_STRLEN) {
        return FAIL_RETURN;
    }
    memcpy(int_id, response->id.value, response->id.value_length);
    id = atoi(int_id);

#if !defined(DM_MESSAGE_CACHE_DISABLED)
    res = dm_msg_cache_search(id, &node);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }
    devid = node->devid;
#endif

    message_len = strlen(DM_MSG_EVENT_SUBDEV_UNREGISTER_REPLY_FMT) + DM_UTILS_UINT32_STRLEN * 3 + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_EVENT_SUBDEV_UNREGISTER_REPLY_FMT, id, response->code.value_int, devid);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_SUBDEV_UNREGISTER_REPLY, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_EVENT_THING_TOPO_ADD_REPLY_FMT[] DM_READ_ONLY = "{\"id\":%d,\"code\":%d,\"devid\":%d}";
int dm_msg_thing_topo_add_reply(dm_msg_response_payload_t *response)
{
    int res = 0, devid = 0, id = 0, message_len = 0;
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};
    char *message = NULL;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_node_t *node = NULL;
#endif

    if (response->id.value_length > DM_UTILS_UINT32_STRLEN) {
        return FAIL_RETURN;
    }
    memcpy(int_id, response->id.value, response->id.value_length);
    id = atoi(int_id);

#if !defined(DM_MESSAGE_CACHE_DISABLED)
    res = dm_msg_cache_search(id, &node);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }
    devid = node->devid;

    /* Update State Machine */
    if (response->code.value_int == IOTX_DM_ERR_CODE_SUCCESS) {
        dm_mgr_set_dev_status(node->devid, IOTX_DM_DEV_STATUS_ATTACHED);
    }

#endif

    message_len = strlen(DM_MSG_EVENT_THING_TOPO_ADD_REPLY_FMT) + DM_UTILS_UINT32_STRLEN * 3 + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_EVENT_THING_TOPO_ADD_REPLY_FMT, id, response->code.value_int, devid);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_TOPO_ADD_REPLY, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_EVENT_THING_TOPO_DELETE_REPLY_FMT[] DM_READ_ONLY = "{\"id\":%d,\"code\":%d,\"devid\":%d}";
int dm_msg_thing_topo_delete_reply(dm_msg_response_payload_t *response)
{
    int res = 0, devid = 0, id = 0, message_len = 0;
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};
    char *message = NULL;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_node_t *node = NULL;
#endif

    if (response->id.value_length > DM_UTILS_UINT32_STRLEN) {
        return FAIL_RETURN;
    }
    memcpy(int_id, response->id.value, response->id.value_length);
    id = atoi(int_id);

#if !defined(DM_MESSAGE_CACHE_DISABLED)
    res = dm_msg_cache_search(id, &node);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }
    devid = node->devid;

    /* Update State Machine */
    if (response->code.value_int == IOTX_DM_ERR_CODE_SUCCESS) {
        dm_mgr_set_dev_status(node->devid, IOTX_DM_DEV_STATUS_ATTACHED);
    }

#endif

    message_len = strlen(DM_MSG_EVENT_THING_TOPO_DELETE_REPLY_FMT) + DM_UTILS_UINT32_STRLEN * 3 + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_EVENT_THING_TOPO_DELETE_REPLY_FMT, id, response->code.value_int, devid);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_TOPO_DELETE_REPLY, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_TOPO_GET_REPLY_FMT[] DM_READ_ONLY = "{\"id\":%d,\"code\":%d,\"devid\":%d,\"topo\":%.*s}";
int dm_msg_topo_get_reply(dm_msg_response_payload_t *response)
{
    int res = 0, id = 0, message_len = 0;
    char *message = NULL;
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};

    if (response == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    /* Message ID */
    if (response->id.value_length > DM_UTILS_UINT32_STRLEN) {
        return FAIL_RETURN;
    }
    memcpy(int_id, response->id.value, response->id.value_length);
    id = atoi(int_id);

    message_len = strlen(DM_MSG_TOPO_GET_REPLY_FMT) + DM_UTILS_UINT32_STRLEN * 3 + response->data.value_length + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_TOPO_GET_REPLY_FMT, id, response->code.value_int, IOTX_DM_LOCAL_NODE_DEVID,
                 response->data.value_length,
                 response->data.value);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_TOPO_GET_REPLY, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

int dm_msg_thing_list_found_reply(dm_msg_response_payload_t *response)
{
    return SUCCESS_RETURN;
}

const char DM_MSG_EVENT_COMBINE_LOGIN_REPLY_FMT[] DM_READ_ONLY = "{\"id\":%d,\"code\":%d,\"devid\":%d}";
int dm_msg_combine_login_reply(dm_msg_response_payload_t *response)
{
    int res = 0, message_len = 0, devid = 0;
    char *message = NULL;
    lite_cjson_t lite, lite_item_pk, lite_item_dk;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};
    char temp_id[DM_UTILS_UINT32_STRLEN] = {0};

    if (response == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }

    /* Parse JSON */
    memset(&lite, 0, sizeof(lite_cjson_t));
    res = lite_cjson_parse(response->data.value, response->data.value_length, &lite);
    if (res != SUCCESS_RETURN) {
        return DM_JSON_PARSE_FAILED;
    }

    /* Parse Product Key */
    res = lite_cjson_object_item(&lite, DM_MSG_KEY_PRODUCT_KEY, strlen(DM_MSG_KEY_PRODUCT_KEY), &lite_item_pk);
    if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_pk)
        || lite_item_pk.value_length >= IOTX_PRODUCT_KEY_LEN + 1) {
        return DM_JSON_PARSE_FAILED;
    }
    memcpy(product_key, lite_item_pk.value, lite_item_pk.value_length);

    /* Parse Device Key */
    res = lite_cjson_object_item(&lite, DM_MSG_KEY_DEVICE_KEY, strlen(DM_MSG_KEY_DEVICE_KEY), &lite_item_dk);
    if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_dk)
        || lite_item_dk.value_length >= IOTX_DEVICE_KEY_LEN + 1) {
        return DM_JSON_PARSE_FAILED;
    }
    memcpy(device_key, lite_item_dk.value, lite_item_dk.value_length);

    /* Get Device Id */
    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    /* Update State Machine */
    if (response->code.value_int == IOTX_DM_ERR_CODE_SUCCESS) {
        dm_mgr_set_dev_status(devid, IOTX_DM_DEV_STATUS_LOGINED);
    }

    /* Message ID */
    memcpy(temp_id, response->id.value, response->id.value_length);

    message_len = strlen(DM_MSG_EVENT_COMBINE_LOGIN_REPLY_FMT) + DM_UTILS_UINT32_STRLEN * 3 + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_EVENT_COMBINE_LOGIN_REPLY_FMT, atoi(temp_id), response->code.value_int,
                 devid);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_COMBINE_LOGIN_REPLY, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    if (response->code.value_int != IOTX_DM_ERR_CODE_SUCCESS) {
        return SUCCESS_RETURN;
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_EVENT_COMBINE_LOGIN_BATCH_REPLY_FMT[] DM_READ_ONLY = "{\"id\":%d,\"code\":%d,\"logined_devids\":[%s],\"failed_devids\":[%s]}";
int dm_msg_combine_login_batch_reply(dm_msg_response_payload_t *response)
{
    int res = 0, message_len = 0, devid = 0, index = 0;
    char *message = NULL;
    lite_cjson_t lite, lite_item_pk, lite_item_dk;
    lite_cjson_t lite_logined_subdevs, lite_failed_subdevs, lite_subdev;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};
    char temp_id[DM_UTILS_UINT32_STRLEN] = {0};

    char *logined_devids = NULL;
    char logined_devids_len = 0;

    char *failed_devids = NULL;
    char failed_devids_len = 0;

    if (response == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }

    /* Parse JSON */
    memset(&lite, 0, sizeof(lite_cjson_t));
    res = lite_cjson_parse(response->data.value, response->data.value_length, &lite);
    if (res != SUCCESS_RETURN) {
        return DM_JSON_PARSE_FAILED;
    }

    /* Parse logined sub devices */
    res = lite_cjson_object_item(&lite, "loginedSubDevices", strlen("loginedSubDevices"), &lite_logined_subdevs);
    if (res != SUCCESS_RETURN || !lite_cjson_is_array(&lite_logined_subdevs)) {
        return DM_JSON_PARSE_FAILED;
    }

    logined_devids_len = lite_logined_subdevs.size * (sizeof(int) + 1);
    logined_devids = DM_malloc(logined_devids_len);
    memset(logined_devids, 0, logined_devids_len);

    for (index = 0; index < lite_logined_subdevs.size; index++) {
        res = lite_cjson_array_item(&lite_logined_subdevs, index, &lite_subdev);
        if (res != SUCCESS_RETURN || !lite_cjson_is_object(&lite_subdev)) {
            DM_free(logined_devids);
            return DM_JSON_PARSE_FAILED;
        }
        
        /* Parse Product Key */
        res = lite_cjson_object_item(&lite_subdev, DM_MSG_KEY_PRODUCT_KEY, strlen(DM_MSG_KEY_PRODUCT_KEY), &lite_item_pk);
        if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_pk)
            || lite_item_pk.value_length >= IOTX_PRODUCT_KEY_LEN + 1) {
            DM_free(logined_devids);
            return DM_JSON_PARSE_FAILED;
        }
        memset(product_key, 0, IOTX_PRODUCT_KEY_LEN + 1);
        memcpy(product_key, lite_item_pk.value, lite_item_pk.value_length);

        /* Parse Device Key */
        res = lite_cjson_object_item(&lite_subdev, DM_MSG_KEY_DEVICE_KEY, strlen(DM_MSG_KEY_DEVICE_KEY), &lite_item_dk);
        if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_dk)
            || lite_item_dk.value_length >= IOTX_DEVICE_KEY_LEN + 1) {
            DM_free(logined_devids);
            return DM_JSON_PARSE_FAILED;
        }
        memset(device_key, 0, IOTX_DEVICE_KEY_LEN + 1);
        memcpy(device_key, lite_item_dk.value, lite_item_dk.value_length);

        /* Get Device Id */
        res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
        if (res != SUCCESS_RETURN) {
            DM_free(logined_devids);
            return FAIL_RETURN;
        }

        /* Update State Machine */
        if (response->code.value_int == IOTX_DM_ERR_CODE_SUCCESS) {
            dm_mgr_set_dev_status(devid, IOTX_DM_DEV_STATUS_LOGINED);
        }
        HAL_Snprintf(logined_devids + strlen(logined_devids), logined_devids_len, "%d", devid);
        if (index < lite_logined_subdevs.size - 1) {
            memcpy(logined_devids + strlen(logined_devids), ",", 1);
        }
    }

    /* Parse failed sub devices */
    res = lite_cjson_object_item(&lite, "failedSubDevices", strlen("failedSubDevices"), &lite_failed_subdevs);
    if (res != SUCCESS_RETURN || !lite_cjson_is_array(&lite_failed_subdevs)) {
        return DM_JSON_PARSE_FAILED;
    }

    failed_devids_len = lite_failed_subdevs.size * (sizeof(int) + 1);
    failed_devids = DM_malloc(failed_devids_len);
    memset(failed_devids, 0, failed_devids_len);

    for (index = 0; index < lite_failed_subdevs.size; index++) {
        res = lite_cjson_array_item(&lite_failed_subdevs, index, &lite_subdev);
        if (res != SUCCESS_RETURN || !lite_cjson_is_object(&lite_subdev)) {
            DM_free(logined_devids);
            DM_free(failed_devids);
            return DM_JSON_PARSE_FAILED;
        }
        
        /* Parse Product Key */
        res = lite_cjson_object_item(&lite_subdev, DM_MSG_KEY_PRODUCT_KEY, strlen(DM_MSG_KEY_PRODUCT_KEY), &lite_item_pk);
        if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_pk)
            || lite_item_pk.value_length >= IOTX_PRODUCT_KEY_LEN + 1) {
            DM_free(logined_devids);
            DM_free(failed_devids);
            return DM_JSON_PARSE_FAILED;
        }
        memset(product_key, 0, IOTX_PRODUCT_KEY_LEN + 1);
        memcpy(product_key, lite_item_pk.value, lite_item_pk.value_length);

        /* Parse Device Key */
        res = lite_cjson_object_item(&lite_subdev, DM_MSG_KEY_DEVICE_KEY, strlen(DM_MSG_KEY_DEVICE_KEY), &lite_item_dk);
        if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_dk)
            || lite_item_dk.value_length >= IOTX_DEVICE_KEY_LEN + 1) {
            DM_free(logined_devids);
            DM_free(failed_devids);
            return DM_JSON_PARSE_FAILED;
        }
        memset(device_key, 0, IOTX_DEVICE_KEY_LEN + 1);
        memcpy(device_key, lite_item_dk.value, lite_item_dk.value_length);

        /* Get Device Id */
        res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
        if (res != SUCCESS_RETURN) {
            DM_free(logined_devids);
            DM_free(failed_devids);
            return FAIL_RETURN;
        }
        HAL_Snprintf(failed_devids + strlen(failed_devids), failed_devids_len, "%d", devid);
        if (index < lite_failed_subdevs.size - 1) {
            memcpy(failed_devids + strlen(failed_devids), ",", 1);
        }
    }

    /* Message ID */
    memcpy(temp_id, response->id.value, response->id.value_length);

    message_len = strlen(DM_MSG_EVENT_COMBINE_LOGIN_BATCH_REPLY_FMT) + DM_UTILS_UINT32_STRLEN * 3 + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        DM_free(logined_devids);
        DM_free(failed_devids);
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_EVENT_COMBINE_LOGIN_BATCH_REPLY_FMT, atoi(temp_id), response->code.value_int,
                 logined_devids, failed_devids);

    DM_free(logined_devids);
    DM_free(failed_devids);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_COMBINE_LOGIN_BATCH_REPLY, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    if (response->code.value_int != IOTX_DM_ERR_CODE_SUCCESS) {
        return SUCCESS_RETURN;
    }

    return SUCCESS_RETURN;
}

const char DM_MSG_EVENT_COMBINE_LOGOUT_REPLY_FMT[] DM_READ_ONLY = "{\"id\":%d,\"code\":%d,\"devid\":%d}";
int dm_msg_combine_logout_reply(dm_msg_response_payload_t *response)
{
    int res = 0, message_len = 0, devid = 0;
    char *message = NULL;
    lite_cjson_t lite, lite_item_pk, lite_item_dk;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};
    char temp_id[DM_UTILS_UINT32_STRLEN] = {0};

    if (response == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    /* Parse JSON */
    memset(&lite, 0, sizeof(lite_cjson_t));
    res = lite_cjson_parse(response->data.value, response->data.value_length, &lite);
    if (res != SUCCESS_RETURN) {
        return DM_JSON_PARSE_FAILED;
    }

    /* Parse Product Key */
    res = lite_cjson_object_item(&lite, DM_MSG_KEY_PRODUCT_KEY, strlen(DM_MSG_KEY_PRODUCT_KEY), &lite_item_pk);
    if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_pk)
        || lite_item_pk.value_length >= IOTX_PRODUCT_KEY_LEN + 1) {
        return DM_JSON_PARSE_FAILED;
    }
    memcpy(product_key, lite_item_pk.value, lite_item_pk.value_length);

    /* Parse Device Key */
    res = lite_cjson_object_item(&lite, DM_MSG_KEY_DEVICE_KEY, strlen(DM_MSG_KEY_DEVICE_KEY), &lite_item_dk);
    if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_dk)
        || lite_item_dk.value_length >= IOTX_DEVICE_KEY_LEN + 1) {
        return DM_JSON_PARSE_FAILED;
    }
    memcpy(device_key, lite_item_dk.value, lite_item_dk.value_length);

    /* Get Device Id */
    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    /* Update State Machine */
    if (response->code.value_int == IOTX_DM_ERR_CODE_SUCCESS) {
        dm_mgr_set_dev_status(devid, IOTX_DM_DEV_STATUS_ATTACHED);
    }

    /* Message ID */
    memcpy(temp_id, response->id.value, response->id.value_length);

    message_len = strlen(DM_MSG_EVENT_COMBINE_LOGOUT_REPLY_FMT) + DM_UTILS_UINT32_STRLEN * 3 + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, DM_MSG_EVENT_COMBINE_LOGOUT_REPLY_FMT, atoi(temp_id), response->code.value_int,
                 devid);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_COMBINE_LOGOUT_REPLY, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

#endif

int dm_msg_cloud_connected(void)
{
    return _dm_msg_send_to_user(IOTX_DM_EVENT_CLOUD_CONNECTED, NULL);
}

int dm_msg_cloud_disconnect(void)
{
    return _dm_msg_send_to_user(IOTX_DM_EVENT_CLOUD_DISCONNECT, NULL);
}

int dm_msg_cloud_reconnect(void)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};

    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_KEY, device_key);

    /* Send To User */
    res = _dm_msg_send_to_user(IOTX_DM_EVENT_CLOUD_RECONNECT, NULL);

    return res;
}

#ifdef DEVICE_MODEL_GATEWAY
const char DM_MSG_THING_SUB_REGISTER_METHOD[] DM_READ_ONLY = "thing.device.register";
const char DM_MSG_THING_SUB_REGISTER_PARAMS[] DM_READ_ONLY = "[{\"productKey\":\"%s\",\"deviceKey\":\"%s\"}]";
int dm_msg_thing_device_register(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                              _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                              _OU_ dm_msg_request_t *request)
{
    int params_len = 0;
    char *params = NULL;

    if (request == NULL || product_key == NULL || device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1) ||
        (strlen(request->product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(request->device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    params_len = strlen(DM_MSG_THING_SUB_REGISTER_PARAMS) + strlen(product_key) + strlen(device_key) + 1;
    params = DM_malloc(params_len);
    if (params == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(params, 0, params_len);
    HAL_Snprintf(params, params_len, DM_MSG_THING_SUB_REGISTER_PARAMS, product_key, device_key);

    /* Get Params */
    request->params = params;
    request->params_len = strlen(request->params);

    /* Get Method */
    request->method = (char *)DM_MSG_THING_SUB_REGISTER_METHOD;

    return SUCCESS_RETURN;
}

const char DM_MSG_THING_TOPO_ADD_SIGN_SOURCE[] DM_READ_ONLY = "clientId%sdeviceKey%sproductKey%stimestamp%s%s";
const char DM_MSG_THING_TOPO_ADD_METHOD[] DM_READ_ONLY = "thing.topo.add";
const char DM_MSG_THING_TOPO_ADD_PARAMS[] DM_READ_ONLY =
            "[{\"productKey\":\"%s\",\"deviceKey\":\"%s\",\"signmethod\":\"%s\",\"sign\":\"%s\",\"timestamp\":\"%s\",\"clientId\":\"%s\"}]";
int dm_msg_thing_topo_add(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                          _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                          _IN_ char device_secret[IOTX_DEVICE_SECRET_LEN + 1], _OU_ dm_msg_request_t *request)
{
    char *params = NULL;
    int params_len = 0;
    char timestamp[DM_UTILS_UINT64_STRLEN] = {0};
    char client_id[IOTX_PRODUCT_KEY_LEN + 1 + IOTX_DEVICE_KEY_LEN + 1 + 1] = {0};
    char *sign_source = NULL;
    int sign_source_len = 0;
    char *sign_method = DM_MSG_SIGN_METHOD_HMACSHA256;
    uint8_t sign[32] = {0};
    char sign_str[65] = {0};


    if (request == NULL || product_key == NULL ||
        device_key == NULL || device_secret == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1) ||
        (strlen(device_secret) >= IOTX_DEVICE_SECRET_LEN + 1) ||
        (strlen(request->product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(request->device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    /* TimeStamp */
    HAL_Snprintf(timestamp, DM_UTILS_UINT64_STRLEN, "%llu", (unsigned long long)HAL_UptimeMs());

    /* Client ID */
    HAL_Snprintf(client_id, IOTX_PRODUCT_KEY_LEN + 1 + IOTX_DEVICE_KEY_LEN + 1 + 1, "%s.%s", product_key, device_key);

    /* Sign */
    sign_source_len = strlen(DM_MSG_THING_TOPO_ADD_SIGN_SOURCE) + strlen(client_id) +
                      strlen(device_key) + strlen(product_key) + strlen(timestamp) + 
                      strlen(device_secret) + 1;
    sign_source = DM_malloc(sign_source_len);
    if (sign_source == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(sign_source, 0, sign_source_len);
    HAL_Snprintf(sign_source, sign_source_len, DM_MSG_THING_TOPO_ADD_SIGN_SOURCE, client_id,
                 device_key, product_key, timestamp, device_secret);

    /*utils_hmac_sha256((uint8_t *)sign_source, strlen(sign_source), (uint8_t *)device_secret, strlen(device_secret), sign);*/
    utils_sha256((uint8_t *)sign_source, strlen(sign_source), sign);
    infra_hex2str(sign, 32, sign_str);

    DM_free(sign_source);

    /* Params */
    request->method = (char *)DM_MSG_THING_TOPO_ADD_METHOD;
    params_len = strlen(DM_MSG_THING_TOPO_ADD_PARAMS) + strlen(product_key) + strlen(device_key) +
                 strlen(sign_method) + strlen(sign_str) + strlen(timestamp) + strlen(client_id) + 1;
    params = DM_malloc(params_len);

    if (params == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(params, 0, params_len);
    HAL_Snprintf(params, params_len, DM_MSG_THING_TOPO_ADD_PARAMS, product_key, device_key,
                 sign_method, sign_str, timestamp, client_id);

    request->params = params;
    request->params_len = strlen(request->params);

    return SUCCESS_RETURN;
}

const char DM_MSG_THING_TOPO_DELETE_METHOD[] DM_READ_ONLY = "thing.topo.delete";
const char DM_MSG_THING_TOPO_DELETE_PARAMS[] DM_READ_ONLY = "[{\"productKey\":\"%s\",\"deviceKey\":\"%s\"}]";
int dm_msg_thing_topo_delete(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                             _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                             _OU_ dm_msg_request_t *request)
{
    char *params = NULL;
    int params_len = 0;

    if (request == NULL || product_key == NULL ||
        device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1) ||
        (strlen(request->product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(request->device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    /* Params */
    request->method = (char *)DM_MSG_THING_TOPO_DELETE_METHOD;
    params_len = strlen(DM_MSG_THING_TOPO_DELETE_PARAMS) + strlen(product_key) + strlen(device_key) + 1;
    params = DM_malloc(params_len);
    if (params == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(params, 0, params_len);
    HAL_Snprintf(params, params_len, DM_MSG_THING_TOPO_DELETE_PARAMS, product_key, device_key);

    request->params = params;
    request->params_len = strlen(request->params);

    return SUCCESS_RETURN;
}

const char DM_MSG_THING_TOPO_GET_METHOD[] DM_READ_ONLY = "thing.topo.get";
const char DM_MSG_THING_TOPO_GET_PARAMS[] DM_READ_ONLY = "{}";
int dm_msg_thing_topo_get(_OU_ dm_msg_request_t *request)
{
    char *params = NULL;
    int params_len = 0;

    /* Params */
    request->method = (char *)DM_MSG_THING_TOPO_GET_METHOD;
    params_len = strlen(DM_MSG_THING_TOPO_GET_PARAMS) + 1;
    params = DM_malloc(params_len);
    if (params == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(params, 0, params_len);
    memcpy(params, DM_MSG_THING_TOPO_GET_PARAMS, strlen(DM_MSG_THING_TOPO_GET_PARAMS));

    request->params = params;
    request->params_len = strlen(request->params);

    return SUCCESS_RETURN;
}

const char DM_MSG_COMBINE_LOGIN_SIGN_SOURCE[] DM_READ_ONLY = "clientId%sdeviceKey%sproductKey%stimestamp%s%s";
const char DM_MSG_COMBINE_LOGIN_METHOD[] DM_READ_ONLY = "combine.login";
const char DM_MSG_COMBINE_LOGIN_PARAMS[] DM_READ_ONLY =
            "{\"productKey\":\"%s\",\"deviceKey\":\"%s\",\"clientId\":\"%s\",\"timestamp\":\"%s\",\"signMethod\":\"%s\",\"sign\":\"%s\",\"cleanSession\":\"%s\"}";
int dm_msg_combine_login(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                         _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                         _IN_ char device_secret[IOTX_DEVICE_SECRET_LEN + 1], _OU_ dm_msg_request_t *request)
{
    char *params = NULL;
    int params_len = 0;
    char timestamp[DM_UTILS_UINT64_STRLEN] = {0};
    char client_id[IOTX_PRODUCT_KEY_LEN + 1 + IOTX_DEVICE_KEY_LEN + 25] = {0};
    char *sign_source = NULL;
    int sign_source_len = 0;
    char *sign_method = DM_MSG_SIGN_METHOD_SHA256;
    uint8_t sign[32] = {0};
    char sign_str[65] = {0};


    if (request == NULL || product_key == NULL ||
        device_key == NULL || device_secret == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1) ||
        (strlen(device_secret) >= IOTX_DEVICE_SECRET_LEN + 1) ||
        (strlen(request->product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(request->device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    /* TimeStamp */
    HAL_Snprintf(timestamp, DM_UTILS_UINT64_STRLEN, "%llu", (unsigned long long)HAL_UptimeMs());

    /* Client ID */
    HAL_Snprintf(client_id, IOTX_PRODUCT_KEY_LEN + 1 + IOTX_DEVICE_KEY_LEN + 25,
                 "%s%s", product_key, device_key);

    /* Sign */
    sign_source_len = strlen(DM_MSG_COMBINE_LOGIN_SIGN_SOURCE) + strlen(client_id) +
                      strlen(device_key) + strlen(product_key) + strlen(timestamp) +
                      strlen(device_secret) + 1;
    sign_source = DM_malloc(sign_source_len);
    if (sign_source == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(sign_source, 0, sign_source_len);
    HAL_Snprintf(sign_source, sign_source_len, DM_MSG_COMBINE_LOGIN_SIGN_SOURCE, client_id,
                 device_key, product_key, timestamp, device_secret);

    /*utils_hmac_sha256((uint8_t *)sign_source, strlen(sign_source), (uint8_t *)device_secret, strlen(device_secret), sign);
    infra_hex2str(sign, 32, sign_str);*/

    utils_sha256((uint8_t *)sign_source, strlen(sign_source), sign);
    infra_hex2str(sign, 32, sign_str);

    DM_free(sign_source);

    /* Params */
    request->method = (char *)DM_MSG_COMBINE_LOGIN_METHOD;
    params_len = strlen(DM_MSG_COMBINE_LOGIN_PARAMS) + strlen(product_key) + strlen(device_key) +
                 strlen(sign_method) + strlen(sign_str) + strlen(timestamp) + strlen(client_id) + 1;
    params = DM_malloc(params_len);

    if (params == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(params, 0, params_len);
    HAL_Snprintf(params, params_len, DM_MSG_COMBINE_LOGIN_PARAMS, product_key, device_key,
                 client_id, timestamp, sign_method, sign_str, "true");

    request->params = params;
    request->params_len = strlen(request->params);

    return SUCCESS_RETURN;
}

const char DM_MSG_COMBINE_LOGIN_BATCH_SIGN_SOURCE[] DM_READ_ONLY = "clientId%sdeviceKey%sproductKey%stimestamp%s%s";
const char DM_MSG_COMBINE_LOGIN_BATCH_METHOD[] DM_READ_ONLY = "combine.login.batch";
const char DM_MSG_COMBINE_LOGIN_BATCH_PARAMS[] DM_READ_ONLY =
            /*"{\"productKey\":\"%s\",\"deviceKey\":\"%s\",\"clientId\":\"%s\",\"timestamp\":\"%s\",\"signMethod\":\"%s\",\"sign\":\"%s\",\"cleanSession\":\"%s\"}";*/
            "{\"clientId\":\"%s\", \"signMethod\":\"%s\", \"timestamp\":\"%s\", \"subDevices\":[%s]}";
const char DM_MSG_SUB_DEVICE_PARAM[] DM_READ_ONLY = "{\"productKey\":\"%s\", \"deviceKey\":\"%s\", \"sign\":\"%s\"}";
int dm_msg_combine_login_batch(_IN_ dm_msg_device_credential_t *device_credentials, int device_credentials_len, _OU_ dm_msg_request_t *request)
{
    char *params = NULL;
    int params_len = 0;
    char timestamp[DM_UTILS_UINT64_STRLEN] = {0};
    char client_id[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char *sign_source = NULL;
    int sign_source_len = 0;
    char *sign_method = DM_MSG_SIGN_METHOD_SHA256;
    uint8_t sign[32] = {0};
    char sign_str[65] = {0};

    /**
     * for store this struct
     * {
     *    "productKey":"x4jwTsoz"
     *    "deviceKey":"xGKFwfkEXz",
     *    "sign":"c14fc21231e6c44849683ccfb7a2089895a278b37a30c33ccb58d3b8690d16e1",
     * }
     */
#define SUB_DEVICE_PARAM_LEN (15 + IOTX_PRODUCT_KEY_LEN + 14 + IOTX_DEVICE_KEY_LEN + 9 + 65 + 4 + 1)
    char sub_device_param[SUB_DEVICE_PARAM_LEN] = {0};
    char *sub_devices_param = NULL;
    int sub_devices_param_len = 0;
    int index = 0;

    char* product_key = NULL;
    char* device_key = NULL;
    char* device_secret = NULL;

    if (request == NULL || device_credentials == NULL || device_credentials_len <= 0 ||
            (strlen(request->product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
            (strlen(request->device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    /* TimeStamp */
    HAL_Snprintf(timestamp, DM_UTILS_UINT64_STRLEN, "%llu", (unsigned long long)HAL_UptimeMs());

    /* Client ID */
    HAL_Snprintf(client_id, IOTX_PRODUCT_KEY_LEN + 1 , "%s", request->product_key);

    /* sub devices */
    sub_devices_param_len = device_credentials_len * SUB_DEVICE_PARAM_LEN + 1;
    sub_devices_param = DM_malloc(sub_devices_param_len);
    memset(sub_devices_param, 0, sub_devices_param_len);
    for (index = 0; index < device_credentials_len; index++)
    {
        dm_msg_device_credential_t device_credential = device_credentials[index];
        product_key = device_credential.product_key;
        device_key = device_credential.device_key;
        device_secret = device_credential.device_secret;

        if (product_key == NULL || device_key == NULL || device_secret == NULL ||
            (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
            (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1) ||
            (strlen(device_secret) >= IOTX_DEVICE_SECRET_LEN + 1)) {
            return STATE_USER_INPUT_INVALID;
        }

        /* Sign */
        sign_source_len = strlen(DM_MSG_COMBINE_LOGIN_BATCH_SIGN_SOURCE) + strlen(client_id) +
                        strlen(device_key) + strlen(product_key) + strlen(timestamp) +
                        strlen(device_secret) + 1;
        sign_source = DM_malloc(sign_source_len);
        if (sign_source == NULL) {
            return STATE_SYS_DEPEND_MALLOC;
        }
        memset(sign_source, 0, sign_source_len);
        HAL_Snprintf(sign_source, sign_source_len, DM_MSG_COMBINE_LOGIN_BATCH_SIGN_SOURCE, client_id,
                    device_key, product_key, timestamp, device_secret);

        utils_sha256((uint8_t *)sign_source, strlen(sign_source), sign);
        infra_hex2str(sign, 32, sign_str);
        DM_free(sign_source);

        HAL_Snprintf(sub_device_param, SUB_DEVICE_PARAM_LEN, DM_MSG_SUB_DEVICE_PARAM, product_key, device_key, sign_str);
        memcpy(sub_devices_param + strlen(sub_devices_param), sub_device_param, strlen(sub_device_param));
        if (index != device_credentials_len - 1)
        {
            memcpy(sub_devices_param + strlen(sub_devices_param), ",", 1);
        }
    }
    

    /* Params */
    request->method = (char *)DM_MSG_COMBINE_LOGIN_BATCH_METHOD;
    params_len = strlen(DM_MSG_COMBINE_LOGIN_BATCH_PARAMS) + strlen(client_id) +
                 strlen(sign_method) + strlen(timestamp) + strlen(sub_devices_param) + 1;
    params = DM_malloc(params_len);

    if (params == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(params, 0, params_len);
    HAL_Snprintf(params, params_len, DM_MSG_COMBINE_LOGIN_BATCH_PARAMS, client_id, sign_method, timestamp, sub_devices_param);
    DM_free(sub_devices_param);

    request->params = params;
    request->params_len = strlen(request->params);

    return SUCCESS_RETURN;
}

const char DM_MSG_COMBINE_LOGOUT_METHOD[] DM_READ_ONLY = "combine.logout";
const char DM_MSG_COMBINE_LOGOUT_PARAMS[] DM_READ_ONLY = "{\"productKey\":\"%s\",\"deviceKey\":\"%s\"}";
int dm_msg_combine_logout(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                          _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                          _OU_ dm_msg_request_t *request)
{
    char *params = NULL;
    int params_len = 0;

    if (product_key == NULL || device_key == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1) ||
        request == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    /* Params */
    request->method = (char *)DM_MSG_COMBINE_LOGOUT_METHOD;
    params_len = strlen(DM_MSG_COMBINE_LOGOUT_PARAMS) + strlen(product_key) + strlen(device_key) + 1;
    params = DM_malloc(params_len);

    if (params == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(params, 0, params_len);
    HAL_Snprintf(params, params_len, DM_MSG_COMBINE_LOGOUT_PARAMS, product_key, device_key);

    request->params = params;
    request->params_len = strlen(request->params);

    return SUCCESS_RETURN;
}
#endif
