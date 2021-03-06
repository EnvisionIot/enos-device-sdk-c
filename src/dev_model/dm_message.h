/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */



#ifndef _DM_MESSAGE_H_
#define _DM_MESSAGE_H_

#include "iotx_dm_internal.h"

#define DM_MSG_KEY_ID                   "id"
#define DM_MSG_KEY_VERSION              "version"
#define DM_MSG_KEY_METHOD               "method"
#define DM_MSG_KEY_PARAMS               "params"
#define DM_MSG_KEY_CODE                 "code"
#define DM_MSG_KEY_DATA                 "data"
#define DM_MSG_KEY_MESSAGE              "message"

#define DM_MSG_VERSION                  "1.0"

#define DM_MSG_KEY_FAILURES             "failures"
#define DM_MSG_KEY_SUCCESSES            "successes"
#define DM_MSG_KEY_ERRORDETAIL          "errorDetail"
#define DM_MSG_KEY_CODE                 "code"
#define DM_MSG_KEY_PRODUCT_KEY          "productKey"
#define DM_MSG_KEY_DEVICE_KEY           "deviceKey"
#define DM_MSG_KEY_DEVICE_SECRET        "deviceSecret"
#define DM_MSG_KEY_TIME                 "time"

#define DM_MSG_SIGN_METHOD_SHA256       "Sha256"
#define DM_MSG_SIGN_METHOD_HMACMD5      "hmacMd5"
#define DM_MSG_SIGN_METHOD_HMACSHA1     "hmacSha1"
#define DM_MSG_SIGN_METHOD_HMACSHA256   "hmacSha256"

typedef enum {
    DM_MSG_DEST_CLOUD = 0x01,
    DM_MSG_DEST_LOCAL = 0x02,
    DM_MSG_DEST_ALL   = 0x03
} dm_msg_dest_type_t;

typedef struct {
    const char *uri;
    unsigned char *payload;
    unsigned int payload_len;
    void *context;
} dm_msg_source_t;

typedef struct {
    const char *uri_name;
} dm_msg_dest_t;

typedef struct {
    lite_cjson_t id;
    lite_cjson_t version;
    lite_cjson_t method;
    lite_cjson_t params;
} dm_msg_request_payload_t;

typedef struct {
    lite_cjson_t id;
    lite_cjson_t code;
    lite_cjson_t data;
    lite_cjson_t message;
} dm_msg_response_payload_t;

typedef struct {
    int msgid;
    int devid;
    const char *service_prefix;
    const char *service_name;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1];
    char device_key[IOTX_DEVICE_KEY_LEN + 1];
    char *params;
    int params_len;
    char *method;
    iotx_cm_data_handle_cb callback;
} dm_msg_request_t;

typedef struct {
    const char *reply_prefix;
    const char *reply_name;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1];
    char device_key[IOTX_DEVICE_KEY_LEN + 1];
    iotx_dm_error_code_t code;
} dm_msg_response_t;

typedef struct {
    int id;
} dm_msg_ctx_t;


int dm_msg_init(void);
int dm_msg_deinit(void);
int _dm_msg_send_to_user(iotx_dm_event_types_t type, char *message);
int dm_msg_send_msg_timeout_to_user(int msg_id, int devid, iotx_dm_event_types_t type);
int dm_msg_uri_parse_pkdk(_IN_ char *uri, _IN_ int uri_len, _IN_ int start_deli, _IN_ int end_deli,
                          _OU_ char product_key[IOTX_PRODUCT_KEY_LEN + 1], _OU_ char device_key[IOTX_DEVICE_KEY_LEN + 1]);
int dm_msg_request_parse(_IN_ char *payload, _IN_ int payload_len, _OU_ dm_msg_request_payload_t *request);
int dm_msg_response_parse(_IN_ char *payload, _IN_ int payload_len, _OU_ dm_msg_response_payload_t *response);
int dm_msg_request(dm_msg_dest_type_t type, _IN_ dm_msg_request_t *request);
int dm_msg_response(dm_msg_dest_type_t type, _IN_ dm_msg_request_payload_t *request, _IN_ dm_msg_response_t *response,
                    _IN_ char *data, _IN_ int data_len, _IN_ void *user_data);
int dm_msg_thing_model_down_raw(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                                _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                                _IN_ char *payload, _IN_ int payload_len);
int dm_msg_thing_model_up_raw_reply(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                                    _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1], char *payload, int payload_len);
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
int dm_msg_measurepoint_set(int devid, dm_msg_request_payload_t *request);
int dm_msg_measurepoint_get(_IN_ int devid, _IN_ dm_msg_request_payload_t *request, _IN_ void *ctx);
int dm_msg_thing_service_invoke(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                                 _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                                 char *identifier, int identifier_len, dm_msg_request_payload_t *request,  _IN_ void *ctx);
int dm_msg_thing_event_measurepoint_post_reply(dm_msg_response_payload_t *response);
int dm_msg_thing_reply(_IN_ char *identifier, _IN_ int identifier_len,
                                  _IN_ dm_msg_response_payload_t *response);
#endif

#ifdef DEVICE_MODEL_GATEWAY
    int dm_msg_thing_disable(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1], _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1]);
    int dm_msg_thing_enable(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1], _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1]);
    int dm_msg_thing_delete(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1], _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1]);
    int dm_msg_combine_disable(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1], _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1]);
    int dm_msg_combine_enable(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1], _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1]);
    int dm_msg_combine_delete(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1], _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1]);
    int dm_msg_thing_device_register_reply(dm_msg_response_payload_t *response);
    int dm_msg_thing_topo_add_reply(dm_msg_response_payload_t *response);
    int dm_msg_thing_topo_delete_reply(dm_msg_response_payload_t *response);
    int dm_msg_topo_get_reply(dm_msg_response_payload_t *response);
    int dm_msg_combine_login_reply(dm_msg_response_payload_t *response);
    int dm_msg_combine_login_batch_reply(dm_msg_response_payload_t *response);
    int dm_msg_combine_logout_reply(dm_msg_response_payload_t *response);
#endif
int dm_msg_cloud_connected(void);
int dm_msg_cloud_disconnect(void);
int dm_msg_cloud_reconnect(void);
#if 0
    int dm_msg_found_device(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
    _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1]);
    int dm_msg_remove_device(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
    _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1]);
    int dm_msg_unregister_result(_IN_ char *uri, _IN_ int result);
    int dm_msg_send_result(_IN_ char *uri, _IN_ int result);
    int dm_msg_add_service_result(_IN_ char *uri, _IN_ int result);
    int dm_msg_remove_service_result(_IN_ char *uri, _IN_ int result);
#endif
int dm_msg_register_result(_IN_ char *uri, _IN_ int result);

#ifdef DEVICE_MODEL_GATEWAY
typedef struct {
    char product_key[IOTX_PRODUCT_KEY_LEN + 1];
    char device_key[IOTX_DEVICE_KEY_LEN + 1];
    char device_secret[IOTX_DEVICE_SECRET_LEN + 1];
} dm_msg_device_credential_t;

int dm_msg_thing_device_register(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                              _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                              _OU_ dm_msg_request_t *request);
int dm_msg_thing_topo_add(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                          _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                          _IN_ char device_secret[IOTX_DEVICE_SECRET_LEN + 1], _OU_ dm_msg_request_t *request);
int dm_msg_thing_topo_delete(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                             _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                             _OU_ dm_msg_request_t *request);
int dm_msg_thing_topo_get(_OU_ dm_msg_request_t *request);
int dm_msg_combine_login(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                         _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                         _IN_ char device_secret[IOTX_DEVICE_SECRET_LEN + 1], _OU_ dm_msg_request_t *request);
                         
int dm_msg_combine_login_batch(_IN_ dm_msg_device_credential_t *device_credentials, int device_credentials_len, _OU_ dm_msg_request_t *request);

int dm_msg_combine_logout(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                          _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                          _OU_ dm_msg_request_t *request);
#endif

int dm_msg_thing_model_user_sub(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN],
                                _IN_ char device_key[IOTX_DEVICE_KEY_LEN],
                                _IN_ char *payload, _IN_ int payload_len);
#endif
