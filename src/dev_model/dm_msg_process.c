/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */



#include "iotx_dm_internal.h"
#include "infra_log.h"

const char DM_URI_SYS_PREFIX[]                        DM_READ_ONLY = "/sys/%s/%s/";
const char DM_URI_EXT_SESSION_PREFIX[]                DM_READ_ONLY = "/ext/session/%s/%s/";
const char DM_URI_REPLY_SUFFIX[]                      DM_READ_ONLY = "_reply";
#if defined(OTA_ENABLED)
const char DM_URI_OTA_DEVICE_INFORM[]                 DM_READ_ONLY = "/ota/device/inform/%s/%s";
#endif

/* From Cloud To Local Request And Response*/
const char DM_URI_THING_MODEL_DOWN_RAW[]              DM_READ_ONLY = "thing/model/down_raw";
const char DM_URI_THING_MODEL_DOWN_RAW_REPLY[]        DM_READ_ONLY = "thing/model/down_raw_reply";

/* From Local To Cloud Request And Response*/
const char DM_URI_THING_MODEL_UP_RAW[]                DM_READ_ONLY = "thing/model/up_raw";
const char DM_URI_THING_MODEL_UP_RAW_REPLY[]          DM_READ_ONLY = "thing/model/up_raw_reply";

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
    /* From Cloud To Local Request And Response*/
    /* EnOS Command - Set Measurepoint */
    const char DM_URI_THING_MEASUREPOINT_SET[]            DM_READ_ONLY = "thing/service/measurepoint/set";
    const char DM_URI_THING_MEASUREPOINT_SET_REPLY[]      DM_READ_ONLY = "thing/service/measurepoint/set_reply";

    /* EnOS Command - Invoke Service */
    const char DM_URI_THING_SERVICE_INVOKE_WILDCARD[]     DM_READ_ONLY = "thing/service/+";
    const char DM_URI_THING_SERVICE_INVOKE_REPLY[]        DM_READ_ONLY = "thing/service/%.*s_reply";

    /* EnOS Tags */
    const char DM_URI_THING_TAG_QUERY[]                   DM_READ_ONLY = "thing/tag/query";
    const char DM_URI_THING_TAG_QUERY_REPLY[]             DM_READ_ONLY = "thing/tag/query_reply";
    const char DM_URI_THING_TAG_UPDATE[]                  DM_READ_ONLY = "thing/tag/update";
    const char DM_URI_THING_TAG_UPDATE_REPLY[]            DM_READ_ONLY = "thing/tag/update_reply";
    const char DM_URI_THING_TAG_DELETE[]                  DM_READ_ONLY = "thing/tag/delete";
    const char DM_URI_THING_TAG_DELETE_REPLY[]            DM_READ_ONLY = "thing/tag/delete_reply";

    /* EnOS ThingModel Attributes */
    const char DM_URI_THING_ATTRIBUTE_QUERY[]             DM_READ_ONLY = "thing/attribute/query";
    const char DM_URI_THING_ATTRIBUTE_QUERY_REPLY[]       DM_READ_ONLY = "thing/attribute/query_reply";
    const char DM_URI_THING_ATTRIBUTE_UPDATE[]            DM_READ_ONLY = "thing/attribute/update";
    const char DM_URI_THING_ATTRIBUTE_UPDATE_REPLY[]      DM_READ_ONLY = "thing/attribute/update_reply";
    const char DM_URI_THING_ATTRIBUTE_DELETE[]            DM_READ_ONLY = "thing/attribute/delete";
    const char DM_URI_THING_ATTRIBUTE_DELETE_REPLY[]      DM_READ_ONLY = "thing/attribute/delete_reply";

    /* EnOS ThingModel Measurepoints */
    const char DM_URI_THING_MEASUREPOINT_POST[]           DM_READ_ONLY = "thing/measurepoint/post";
    const char DM_URI_THING_MEASUREPOINT_POST_REPLY[]     DM_READ_ONLY = "thing/measurepoint/post_reply";
    const char DM_URI_THING_MEASUREPOINT_POST_BATCH[]           DM_READ_ONLY = "thing/measurepoint/post/batch";
    const char DM_URI_THING_MEASUREPOINT_POST_BATCH_REPLY[]     DM_READ_ONLY = "thing/measurepoint/post/batch_reply";

#ifdef DEVICE_MEASUREPOINT_RESUME
    /* EnOS ThingModel Measurepoints Resume */
    const char DM_URI_THING_MEASUREPOINT_RESUME[]          DM_READ_ONLY = "thing/measurepoint/resume";
    const char DM_URI_THING_MEASUREPOINT_RESUME_REPLY[]    DM_READ_ONLY = "thing/measurepoint/resume_reply";
    const char DM_URI_THING_MEASUREPOINT_RESUME_BATCH[]          DM_READ_ONLY = "thing/measurepoint/resume/batch";
    const char DM_URI_THING_MEASUREPOINT_RESUME_BATCH_REPLY[]    DM_READ_ONLY = "thing/measurepoint/resume/batch_reply";
#endif /* #ifdef DEVICE_MEASUREPOINT_RESUME */

    /* EnOS ThingModel Events */
    const char DM_URI_THING_EVENT_POST[]                  DM_READ_ONLY = "thing/event/%.*s/post";
    const char DM_URI_THING_EVENT_POST_REPLY_WILDCARD[]   DM_READ_ONLY = "thing/event/+/post_reply";


#endif

#ifdef DEVICE_MODEL_GATEWAY
    /* From Cloud To Local Request And Response*/
    const char DM_URI_THING_DELETE[]                      DM_READ_ONLY = "thing/delete";
    const char DM_URI_THING_DELETE_REPLY[]                DM_READ_ONLY = "thing/delete_reply";
    const char DM_URI_THING_DISABLE[]                     DM_READ_ONLY = "thing/disable";
    const char DM_URI_THING_DISABLE_REPLY[]               DM_READ_ONLY = "thing/disable_reply";
    const char DM_URI_THING_ENABLE[]                      DM_READ_ONLY = "thing/enable";
    const char DM_URI_THING_ENABLE_REPLY[]                DM_READ_ONLY = "thing/enable_reply";

    const char DM_URI_COMBINE_DELETE[]                      DM_READ_ONLY = "combine/delete";
    const char DM_URI_COMBINE_DELETE_REPLY[]                DM_READ_ONLY = "combine/delete_reply";
    const char DM_URI_COMBINE_DISABLE[]                     DM_READ_ONLY = "combine/disable";
    const char DM_URI_COMBINE_DISABLE_REPLY[]               DM_READ_ONLY = "combine/disable_reply";
    const char DM_URI_COMBINE_ENABLE[]                      DM_READ_ONLY = "combine/enable";
    const char DM_URI_COMBINE_ENABLE_REPLY[]                DM_READ_ONLY = "combine/enable_reply";

    /* From Local To Cloud Request And Response*/
    const char DM_URI_THING_DEVICE_REGISTER[]              DM_READ_ONLY = "thing/device/register";
    const char DM_URI_THING_DEVICE_REGISTER_REPLY[]        DM_READ_ONLY = "thing/device/register_reply";
    const char DM_URI_THING_TOPO_ADD[]                     DM_READ_ONLY = "thing/topo/add";
    const char DM_URI_THING_TOPO_ADD_REPLY[]               DM_READ_ONLY = "thing/topo/add_reply";
    const char DM_URI_THING_TOPO_DELETE[]                  DM_READ_ONLY = "thing/topo/delete";
    const char DM_URI_THING_TOPO_DELETE_REPLY[]            DM_READ_ONLY = "thing/topo/delete_reply";
    const char DM_URI_THING_TOPO_GET[]                     DM_READ_ONLY = "thing/topo/get";
    const char DM_URI_THING_TOPO_GET_REPLY[]               DM_READ_ONLY = "thing/topo/get_reply";
    const char DM_URI_COMBINE_LOGIN[]                      DM_READ_ONLY = "combine/login";
    const char DM_URI_COMBINE_LOGIN_REPLY[]                DM_READ_ONLY = "combine/login_reply";
    const char DM_URI_COMBINE_LOGIN_BATCH[]                DM_READ_ONLY = "combine/login/batch";
    const char DM_URI_COMBINE_LOGIN_BATCH_REPLY[]          DM_READ_ONLY = "combine/login/batch_reply";
    const char DM_URI_COMBINE_LOGOUT[]                     DM_READ_ONLY = "combine/logout";
    const char DM_URI_COMBINE_LOGOUT_REPLY[]               DM_READ_ONLY = "combine/logout_reply";
#endif

int dm_msg_proc_thing_model_down_raw(_IN_ dm_msg_source_t *source)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};

    /* Parse Product Key And Device Key */
    res = dm_msg_uri_parse_pkdk((char *)source->uri, strlen(source->uri), 2 + DM_URI_OFFSET, 4 + DM_URI_OFFSET, product_key,
                                device_key);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    return dm_msg_thing_model_down_raw(product_key, device_key, (char *)source->payload, source->payload_len);
}

int dm_msg_proc_thing_model_up_raw_reply(_IN_ dm_msg_source_t *source)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_MODEL_UP_RAW_REPLY);

    res = dm_msg_uri_parse_pkdk((char *)source->uri, strlen(source->uri), 2 + DM_URI_OFFSET, 4 + DM_URI_OFFSET, product_key,
                                device_key);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    res = dm_msg_thing_model_up_raw_reply(product_key, device_key, (char *)source->payload, source->payload_len);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    return SUCCESS_RETURN;
}

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
int dm_msg_proc_thing_measurepoint_set(_IN_ dm_msg_source_t *source, _IN_ dm_msg_dest_t *dest,
        _OU_ dm_msg_request_payload_t *request, _OU_ dm_msg_response_t *response)
{
    int res = 0, devid = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_MEASUREPOINT_SET);

    /* Request */
    res = dm_msg_uri_parse_pkdk((char *)source->uri, strlen(source->uri), 2 + DM_URI_OFFSET, 4 + DM_URI_OFFSET, product_key,
                                device_key);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    res = dm_mgr_search_device_by_pkdk(product_key, device_key, &devid);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    res = dm_msg_request_parse((char *)source->payload, source->payload_len, request);
    if (res < SUCCESS_RETURN) {
        return res ;
    }

    /* Operation */
    res = dm_msg_measurepoint_set(devid, request);

    /* Response */
    response->reply_prefix = DM_URI_SYS_PREFIX;
    response->reply_name = dest->uri_name;
    memcpy(response->product_key, product_key, strlen(product_key));
    memcpy(response->device_key, device_key, strlen(device_key));
    response->code = (res == SUCCESS_RETURN) ? (IOTX_DM_ERR_CODE_SUCCESS) : (IOTX_DM_ERR_CODE_REQUEST_ERROR);

    return SUCCESS_RETURN;
}

int dm_msg_proc_thing_service_invoke(_IN_ dm_msg_source_t *source)
{
    int res = 0, serviceid_pos = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};
    dm_msg_request_payload_t request;

    memset(&request, 0, sizeof(dm_msg_request_payload_t));

    res = dm_utils_memtok((char *)source->uri, strlen(source->uri), DM_URI_SERVICE_DELIMITER, 6, &serviceid_pos);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Parse Product Key And Device Key */
    res = dm_msg_uri_parse_pkdk((char *)source->uri, strlen(source->uri), 2 + DM_URI_OFFSET, 4 + DM_URI_OFFSET, product_key,
                                device_key);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Request */
    res = dm_msg_request_parse((char *)source->payload, source->payload_len, &request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    return dm_msg_thing_service_invoke(product_key, device_key, (char *)source->uri + serviceid_pos + 1,
                                        strlen(source->uri) - serviceid_pos - 1, &request, source->context);
}

int dm_msg_proc_thing_reply(_IN_ dm_msg_source_t *source)
{
    int res = 0, method_start_pos = 0, method_end_pos = 0;
    dm_msg_response_payload_t response;
    char *method_replace_pos;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};
#endif

    res = dm_utils_memtok((char *)source->uri, strlen(source->uri), DM_URI_SERVICE_DELIMITER, 4 + DM_URI_OFFSET,
                          &method_start_pos);
    if (res != SUCCESS_RETURN) {
        return STATE_DEV_MODEL_URL_SPLIT_FAILED;
    }

    method_end_pos = strlen(source->uri);

    /* EnOS: Replace / with . to generate method */
    do {
        method_replace_pos = strchr(source->uri, DM_URI_SERVICE_DELIMITER);
        if (method_replace_pos != NULL) {
            *method_replace_pos = DM_ENOS_METHOD_DELIMITER;
        }
    } while (method_replace_pos != NULL);

    log_debug("dm_msg", "method %.*s", method_end_pos - method_start_pos - 1,
            (char *) source->uri + method_start_pos + 1);

    /* Response */
    res = dm_msg_response_parse((char *)source->payload, source->payload_len, &response);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    dm_msg_thing_reply((char *)source->uri + method_start_pos + 1, method_end_pos - method_start_pos - 1,
                                      &response);

    /* Remove Message From Cache */
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (response.id.value_length > DM_UTILS_UINT32_STRLEN) {
        return STATE_DEV_MODEL_WRONG_JSON_FORMAT;
    }
    memcpy(int_id, response.id.value, response.id.value_length);
    dm_msg_cache_remove(atoi(int_id));
#endif
    return SUCCESS_RETURN;
}

#endif

#ifdef DEVICE_MODEL_GATEWAY
int dm_msg_proc_thing_disable(_IN_ dm_msg_source_t *source, _IN_ dm_msg_dest_t *dest,
                              _OU_ dm_msg_request_payload_t *request, _OU_ dm_msg_response_t *response)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_DISABLE);

    /* Request */
    res = dm_msg_uri_parse_pkdk((char *)source->uri, strlen(source->uri), 2 + DM_URI_OFFSET, 4 + DM_URI_OFFSET, product_key,
                                device_key);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    res = dm_msg_request_parse((char *)source->payload, source->payload_len, request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    res = dm_msg_thing_disable(product_key, device_key);

    /* Response */
    response->reply_prefix = DM_URI_SYS_PREFIX;
    response->reply_name = dest->uri_name;
    memcpy(response->product_key, product_key, strlen(product_key));
    memcpy(response->device_key, device_key, strlen(device_key));
    response->code = (res == SUCCESS_RETURN) ? (IOTX_DM_ERR_CODE_SUCCESS) : (IOTX_DM_ERR_CODE_REQUEST_ERROR);

    return SUCCESS_RETURN;
}

int dm_msg_proc_thing_enable(_IN_ dm_msg_source_t *source, _IN_ dm_msg_dest_t *dest,
                             _OU_ dm_msg_request_payload_t *request, _OU_ dm_msg_response_t *response)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_DISABLE);

    /* Request */
    res = dm_msg_uri_parse_pkdk((char *)source->uri, strlen(source->uri), 2 + DM_URI_OFFSET, 4 + DM_URI_OFFSET, product_key,
                                device_key);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    res = dm_msg_request_parse((char *)source->payload, source->payload_len, request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    res = dm_msg_thing_enable(product_key, device_key);

    /* Response */
    response->reply_prefix = DM_URI_SYS_PREFIX;
    response->reply_name = dest->uri_name;
    memcpy(response->product_key, product_key, strlen(product_key));
    memcpy(response->device_key, device_key, strlen(device_key));
    response->code = (res == SUCCESS_RETURN) ? (IOTX_DM_ERR_CODE_SUCCESS) : (IOTX_DM_ERR_CODE_REQUEST_ERROR);

    return SUCCESS_RETURN;
}

int dm_msg_proc_thing_delete(_IN_ dm_msg_source_t *source, _IN_ dm_msg_dest_t *dest,
                             _OU_ dm_msg_request_payload_t *request, _OU_ dm_msg_response_t *response)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_DELETE);

    /* Request */
    res = dm_msg_uri_parse_pkdk((char *)source->uri, strlen(source->uri), 2 + DM_URI_OFFSET, 4 + DM_URI_OFFSET, product_key,
                                device_key);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    res = dm_msg_request_parse((char *)source->payload, source->payload_len, request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    res = dm_msg_thing_delete(product_key, device_key);

    /* Response */
    response->reply_prefix = DM_URI_SYS_PREFIX;
    response->reply_name = dest->uri_name;
    memcpy(response->product_key, product_key, strlen(product_key));
    memcpy(response->device_key, device_key, strlen(device_key));
    response->code = (res == SUCCESS_RETURN) ? (IOTX_DM_ERR_CODE_SUCCESS) : (IOTX_DM_ERR_CODE_REQUEST_ERROR);

    return SUCCESS_RETURN;
}

int dm_msg_proc_combine_disable(_IN_ dm_msg_source_t *source, _IN_ dm_msg_dest_t *dest,
                              _OU_ dm_msg_request_payload_t *request, _OU_ dm_msg_response_t *response)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};
    lite_cjson_t lite_product_key;
    lite_cjson_t lite_device_key;

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_DISABLE);

    res = dm_msg_request_parse((char *)source->payload, source->payload_len, request);
    if (res != SUCCESS_RETURN) {
        return res;
    }
    
    res = lite_cjson_object_item(&request->params, DM_MSG_KEY_PRODUCT_KEY, strlen(DM_MSG_KEY_PRODUCT_KEY), &lite_product_key);
    if (res != SUCCESS_RETURN) {
        return res;
    }
    res = lite_cjson_object_item(&request->params, DM_MSG_KEY_DEVICE_KEY, strlen(DM_MSG_KEY_DEVICE_KEY), &lite_device_key);
    if (res != SUCCESS_RETURN) {
        return res;
    }
    memcpy(product_key, lite_product_key.value, lite_product_key.value_length);
    memcpy(device_key, lite_device_key.value, lite_device_key.value_length);

    /* Operation */
    res = dm_msg_combine_disable(product_key, device_key);

    /* pk and dk for Response to cloud */
    memset(product_key, 0, IOTX_PRODUCT_KEY_LEN + 1);
    memset(device_key, 0, IOTX_DEVICE_KEY_LEN + 1);
    res = dm_msg_uri_parse_pkdk((char *)source->uri, strlen(source->uri), 2 + DM_URI_OFFSET, 4 + DM_URI_OFFSET, product_key,
                                device_key);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    /* Response */
    response->reply_prefix = DM_URI_EXT_SESSION_PREFIX;
    response->reply_name = dest->uri_name;
    memcpy(response->product_key, product_key, strlen(product_key));
    memcpy(response->device_key, device_key, strlen(device_key));
    response->code = (res == SUCCESS_RETURN) ? (IOTX_DM_ERR_CODE_SUCCESS) : (IOTX_DM_ERR_CODE_REQUEST_ERROR);

    return SUCCESS_RETURN;
}

int dm_msg_proc_combine_enable(_IN_ dm_msg_source_t *source, _IN_ dm_msg_dest_t *dest,
                              _OU_ dm_msg_request_payload_t *request, _OU_ dm_msg_response_t *response)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};
    lite_cjson_t lite_product_key;
    lite_cjson_t lite_device_key;

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_DISABLE);

    res = dm_msg_request_parse((char *)source->payload, source->payload_len, request);
    if (res != SUCCESS_RETURN) {
        return res;
    }
    
    res = lite_cjson_object_item(&request->params, DM_MSG_KEY_PRODUCT_KEY, strlen(DM_MSG_KEY_PRODUCT_KEY), &lite_product_key);
    if (res != SUCCESS_RETURN) {
        return res;
    }
    res = lite_cjson_object_item(&request->params, DM_MSG_KEY_DEVICE_KEY, strlen(DM_MSG_KEY_DEVICE_KEY), &lite_device_key);
    if (res != SUCCESS_RETURN) {
        return res;
    }
    memcpy(product_key, lite_product_key.value, lite_product_key.value_length);
    memcpy(device_key, lite_device_key.value, lite_device_key.value_length);

    /* Operation */
    res = dm_msg_combine_enable(product_key, device_key);

    /* pk and dk for Response to cloud */
    memset(product_key, 0, IOTX_PRODUCT_KEY_LEN + 1);
    memset(device_key, 0, IOTX_DEVICE_KEY_LEN + 1);
    res = dm_msg_uri_parse_pkdk((char *)source->uri, strlen(source->uri), 2 + DM_URI_OFFSET, 4 + DM_URI_OFFSET, product_key,
                                device_key);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    /* Response */
    response->reply_prefix = DM_URI_EXT_SESSION_PREFIX;
    response->reply_name = dest->uri_name;
    memcpy(response->product_key, product_key, strlen(product_key));
    memcpy(response->device_key, device_key, strlen(device_key));
    response->code = (res == SUCCESS_RETURN) ? (IOTX_DM_ERR_CODE_SUCCESS) : (IOTX_DM_ERR_CODE_REQUEST_ERROR);

    return SUCCESS_RETURN;
}

int dm_msg_proc_combine_delete(_IN_ dm_msg_source_t *source, _IN_ dm_msg_dest_t *dest,
                              _OU_ dm_msg_request_payload_t *request, _OU_ dm_msg_response_t *response)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};
    lite_cjson_t lite_product_key;
    lite_cjson_t lite_device_key;

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_DISABLE);

    res = dm_msg_request_parse((char *)source->payload, source->payload_len, request);
    if (res != SUCCESS_RETURN) {
        return res;
    }
    
    res = lite_cjson_object_item(&request->params, DM_MSG_KEY_PRODUCT_KEY, strlen(DM_MSG_KEY_PRODUCT_KEY), &lite_product_key);
    if (res != SUCCESS_RETURN) {
        return res;
    }
    res = lite_cjson_object_item(&request->params, DM_MSG_KEY_DEVICE_KEY, strlen(DM_MSG_KEY_DEVICE_KEY), &lite_device_key);
    if (res != SUCCESS_RETURN) {
        return res;
    }
    memcpy(product_key, lite_product_key.value, lite_product_key.value_length);
    memcpy(device_key, lite_device_key.value, lite_device_key.value_length);

    /* Operation */
    res = dm_msg_combine_delete(product_key, device_key);

    /* pk and dk for Response to cloud */
    memset(product_key, 0, IOTX_PRODUCT_KEY_LEN + 1);
    memset(device_key, 0, IOTX_DEVICE_KEY_LEN + 1);
    res = dm_msg_uri_parse_pkdk((char *)source->uri, strlen(source->uri), 2 + DM_URI_OFFSET, 4 + DM_URI_OFFSET, product_key,
                                device_key);
    if (res < SUCCESS_RETURN) {
        return res;
    }
    
    /* Response */
    response->reply_prefix = DM_URI_EXT_SESSION_PREFIX;
    response->reply_name = dest->uri_name;
    memcpy(response->product_key, product_key, strlen(product_key));
    memcpy(response->device_key, device_key, strlen(device_key));
    response->code = (res == SUCCESS_RETURN) ? (IOTX_DM_ERR_CODE_SUCCESS) : (IOTX_DM_ERR_CODE_REQUEST_ERROR);

    return SUCCESS_RETURN;
}

int dm_msg_proc_thing_device_register_reply(_IN_ dm_msg_source_t *source)
{
    int res = 0;
    dm_msg_response_payload_t response;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};
#endif

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_DEVICE_REGISTER_REPLY);

    memset(&response, 0, sizeof(dm_msg_response_payload_t));

    /* Response */
    res = dm_msg_response_parse((char *)source->payload, source->payload_len, &response);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    dm_msg_thing_device_register_reply(&response);

    /* Remove Message From Cache */
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (response.id.value_length > DM_UTILS_UINT32_STRLEN) {
        return STATE_DEV_MODEL_WRONG_JSON_FORMAT;
    }
    memcpy(int_id, response.id.value, response.id.value_length);
    dm_msg_cache_remove(atoi(int_id));
#endif

    return SUCCESS_RETURN;
}

int dm_msg_proc_thing_topo_add_reply(_IN_ dm_msg_source_t *source)
{
    int res = 0;
    dm_msg_response_payload_t response;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};
#endif

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_TOPO_ADD_REPLY);

    memset(&response, 0, sizeof(dm_msg_response_payload_t));

    /* Response */
    res = dm_msg_response_parse((char *)source->payload, source->payload_len, &response);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    dm_msg_thing_topo_add_reply(&response);

    /* Remove Message From Cache */
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (response.id.value_length > DM_UTILS_UINT32_STRLEN) {
        return STATE_DEV_MODEL_WRONG_JSON_FORMAT;
    }
    memcpy(int_id, response.id.value, response.id.value_length);
    dm_msg_cache_remove(atoi(int_id));
#endif
    return SUCCESS_RETURN;
}

int dm_msg_proc_thing_topo_delete_reply(_IN_ dm_msg_source_t *source)
{
    int res = 0;
    dm_msg_response_payload_t response;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};
#endif

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_TOPO_DELETE_REPLY);

    memset(&response, 0, sizeof(dm_msg_response_payload_t));

    /* Response */
    res = dm_msg_response_parse((char *)source->payload, source->payload_len, &response);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    dm_msg_thing_topo_delete_reply(&response);

    /* Remove Message From Cache */
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (response.id.value_length > DM_UTILS_UINT32_STRLEN) {
        return STATE_DEV_MODEL_WRONG_JSON_FORMAT;
    }
    memcpy(int_id, response.id.value, response.id.value_length);
    dm_msg_cache_remove(atoi(int_id));
#endif
    return SUCCESS_RETURN;
}

int dm_msg_proc_thing_topo_get_reply(_IN_ dm_msg_source_t *source)
{
    int res = 0;
    dm_msg_response_payload_t response;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};
#endif

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_TOPO_GET_REPLY);

    memset(&response, 0, sizeof(dm_msg_response_payload_t));

    /* Response */
    res = dm_msg_response_parse((char *)source->payload, source->payload_len, &response);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    dm_msg_topo_get_reply(&response);

    /* Remove Message From Cache */
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (response.id.value_length > DM_UTILS_UINT32_STRLEN) {
        return STATE_DEV_MODEL_WRONG_JSON_FORMAT;
    }
    memcpy(int_id, response.id.value, response.id.value_length);
    dm_msg_cache_remove(atoi(int_id));
#endif
    return SUCCESS_RETURN;
}

int dm_msg_proc_combine_login_reply(_IN_ dm_msg_source_t *source)
{
    int res = 0;
    dm_msg_response_payload_t response;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};
#endif

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_TOPO_GET_REPLY);

    memset(&response, 0, sizeof(dm_msg_response_payload_t));

    /* Response */
    res = dm_msg_response_parse((char *)source->payload, source->payload_len, &response);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    dm_msg_combine_login_reply(&response);

    /* Remove Message From Cache */
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (response.id.value_length > DM_UTILS_UINT32_STRLEN) {
        return STATE_DEV_MODEL_WRONG_JSON_FORMAT;
    }
    memcpy(int_id, response.id.value, response.id.value_length);
    dm_msg_cache_remove(atoi(int_id));
#endif
    return SUCCESS_RETURN;
}

int dm_msg_proc_combine_login_batch_reply(_IN_ dm_msg_source_t *source)
{
    int res = 0;
    dm_msg_response_payload_t response;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};
#endif

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_TOPO_GET_REPLY);

    memset(&response, 0, sizeof(dm_msg_response_payload_t));

    /* Response */
    res = dm_msg_response_parse((char *)source->payload, source->payload_len, &response);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    dm_msg_combine_login_batch_reply(&response);

    /* Remove Message From Cache */
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (response.id.value_length > DM_UTILS_UINT32_STRLEN) {
        return STATE_DEV_MODEL_WRONG_JSON_FORMAT;
    }
    memcpy(int_id, response.id.value, response.id.value_length);
    dm_msg_cache_remove(atoi(int_id));
#endif
    return SUCCESS_RETURN;
}

int dm_msg_proc_combine_logout_reply(_IN_ dm_msg_source_t *source)
{
    int res = 0;
    dm_msg_response_payload_t response;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    char int_id[DM_UTILS_UINT32_STRLEN + 1] = {0};
#endif

    iotx_state_event(ITE_STATE_DEV_MODEL, STATE_DEV_MODEL_RX_CLOUD_MESSAGE, DM_URI_THING_TOPO_GET_REPLY);

    memset(&response, 0, sizeof(dm_msg_response_payload_t));

    /* Response */
    res = dm_msg_response_parse((char *)source->payload, source->payload_len, &response);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Operation */
    dm_msg_combine_logout_reply(&response);

    /* Remove Message From Cache */
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (response.id.value_length > DM_UTILS_UINT32_STRLEN) {
        return STATE_DEV_MODEL_WRONG_JSON_FORMAT;
    }
    memcpy(int_id, response.id.value, response.id.value_length);
    dm_msg_cache_remove(atoi(int_id));
#endif
    return SUCCESS_RETURN;
}
#endif

int dm_msg_proc_thing_model_user_sub(_IN_ dm_msg_source_t *source)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN] = {0};

    /* Parse Product Key And Device Key */
    res = dm_msg_uri_parse_pkdk((char *)source->uri, strlen(source->uri), 2 + DM_URI_OFFSET, 4 + DM_URI_OFFSET, product_key,
                                device_key);
    if (res != SUCCESS_RETURN) {
        return res;
    }
    return dm_msg_thing_model_user_sub(product_key, device_key, (char *)source->payload, source->payload_len);
}
