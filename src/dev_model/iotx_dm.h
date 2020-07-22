/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */



#ifndef _IOT_EXPORT_DM_H_
#define _IOT_EXPORT_DM_H_

#ifndef _IN_
    #define _IN_
#endif

#ifndef _OU_
    #define _OU_
#endif

#ifdef DEVICE_MODEL_GATEWAY
    #define IOTX_DM_DEVICE_TYPE IOTX_DM_DEVICE_GATEWAY
#else
    #define IOTX_DM_DEVICE_TYPE IOTX_DM_DEVICE_SINGLE
#endif

#define IOTX_DM_LOCAL_NODE_DEVID (0)

#define IOTX_DM_DEVICE_SINGLE  (0x01)
#define IOTX_DM_DEVICE_SUBDEV  (0x02)
#define IOTX_DM_DEVICE_GATEWAY (0x04)
#define IOTX_DM_DEVICE_MAIN    (IOTX_DM_DEVICE_SINGLE|IOTX_DM_DEVICE_GATEWAY)
#define IOTX_DM_DEVICE_ALL     (IOTX_DM_DEVICE_SINGLE|IOTX_DM_DEVICE_SUBDEV|IOTX_DM_DEVICE_GATEWAY)

/* Service Type 0~7bit: type, 8~15bit: extended*/
#define IOTX_DM_SERVICE_CLOUD         (0x0001)
#define IOTX_DM_SERVICE_LOCAL         (0x0002)
#define IOTX_DM_SERVICE_LOCAL_NO_AUTH (0x0000)
#define IOTX_DM_SERVICE_LOCAL_AUTH    (0x0100)

#define IOTX_DM_LOCAL_AUTH            (IOTX_DM_SERVICE_LOCAL|IOTX_DM_SERVICE_LOCAL_AUTH)
#define IOTX_DM_LOCAL_NO_AUTH         (IOTX_DM_SERVICE_LOCAL|IOTX_DM_SERVICE_LOCAL_NO_AUTH)

#define IOTX_DM_SERVICE_ALL           (IOTX_DM_SERVICE_CLOUD|IOTX_DM_LOCAL_AUTH)

typedef enum {
    IOTX_DM_ERR_CODE_SUCCESS              = 200,
    IOTX_DM_ERR_CODE_REQUEST_ERROR        = 400,
    IOTX_DM_ERR_CODE_REQUEST_PARAMS_ERROR = 460,
    IOTX_DM_ERR_CODE_REQUEST_TOO_MANY     = 429,
    IOTX_DM_ERR_CODE_NO_ACTIVE_SESSION    = 520,
    IOTX_DM_ERR_CODE_TIMEOUT              = 100000
} iotx_dm_error_code_t;

typedef enum {
    IOTX_DM_EVENT_CLOUD_CONNECTED  = 0,
    IOTX_DM_EVENT_CLOUD_DISCONNECT,
    IOTX_DM_EVENT_CLOUD_RECONNECT,
    IOTX_DM_EVENT_LOCAL_CONNECTED,
    IOTX_DM_EVENT_LOCAL_DISCONNECT,
    IOTX_DM_EVENT_LOCAL_RECONNECT,
    IOTX_DM_EVENT_FOUND_DEVICE,
    IOTX_DM_EVENT_REMOVE_DEVICE,
    IOTX_DM_EVENT_REGISTER_RESULT,
    IOTX_DM_EVENT_UNREGISTER_RESULT,
    IOTX_DM_EVENT_INITIALIZED,
    IOTX_DM_EVENT_SEND_RESULT,
    IOTX_DM_EVENT_ADD_SERVICE_RESULT,
    IOTX_DM_EVENT_REMOVE_SERVICE_RESULT,
    IOTX_DM_EVENT_NEW_DATA_RECEIVED,
    IOTX_DM_EVENT_MEASUREPOINT_SET,
    IOTX_DM_EVENT_TOPO_ADD_NOTIFY,
    IOTX_DM_EVENT_THING_SERVICE_INVOKE,
    IOTX_DM_EVENT_THING_DISABLE,
    IOTX_DM_EVENT_THING_ENABLE,
    IOTX_DM_EVENT_THING_DELETE,
    IOTX_DM_EVENT_MODEL_DOWN_RAW,
    IOTX_DM_EVENT_GATEWAY_PERMIT,
    IOTX_DM_EVENT_SUBDEV_REGISTER_REPLY,
    IOTX_DM_EVENT_PROXY_PRODUCT_REGISTER_REPLY,
    IOTX_DM_EVENT_SUBDEV_UNREGISTER_REPLY,
    IOTX_DM_EVENT_TOPO_ADD_REPLY,
    IOTX_DM_EVENT_TOPO_DELETE_REPLY,
    IOTX_DM_EVENT_TOPO_GET_REPLY,
    IOTX_DM_EVENT_TOPO_ADD_NOTIFY_REPLY,
    IOTX_DM_EVENT_THING_REPLY,
    IOTX_DM_EVENT_COMBINE_LOGIN_REPLY,
    IOTX_DM_EVENT_COMBINE_LOGIN_BATCH_REPLY,
    IOTX_DM_EVENT_COMBINE_LOGOUT_REPLY,
    IOTX_DM_EVENT_COMBINE_DISABLE,
    IOTX_DM_EVENT_COMBINE_ENABLE,
    IOTX_DM_EVENT_COMBINE_DELETE,
    IOTX_DM_EVENT_MODEL_UP_RAW_REPLY,
    IOTX_DM_EVENT_CLOUD_ERROR,
    IOTX_DM_EVENT_MAX
} iotx_dm_event_types_t;

typedef void (*iotx_dm_event_callback)(iotx_dm_event_types_t type, char *payload);

typedef enum {
    IOTX_DM_DEVICE_SECRET_PRODUCT,
    IOTX_DM_DEVICE_SECRET_DEVICE,
    IOTX_DM_DEVICE_SECRET_TYPES_MAX
} iotx_dm_device_secret_types_t;

typedef enum {
    IOTX_DM_MESSAGE_NO_AUTH,
    IOTX_DM_MESSAGE_AUTH,
    IOTX_DM_MESSAGE_AUTH_MAX
} iotx_dm_message_auth_types_t;

typedef enum {
    IOTX_DM_TSL_SOURCE_LOCAL,
    IOTX_DM_TSL_SOURCE_CLOUD
} iotx_dm_tsl_source_t;

typedef struct {
    iotx_dm_device_secret_types_t secret_type;
    iotx_dm_event_callback event_callback;
} iotx_dm_init_params_t;

typedef enum {
    IOTX_DM_DEV_AVAIL_ENABLE,
    IOTX_DM_DEV_AVAIL_DISABLE
} iotx_dm_dev_avail_t;

typedef enum {
    IOTX_DM_DEV_STATUS_UNAUTHORIZED,   /* Subdev Created */
    IOTX_DM_DEV_STATUS_AUTHORIZED,     /* Receive Topo Add Notify */
    IOTX_DM_DEV_STATUS_REGISTERED,     /* Receive Subdev Registered */
    IOTX_DM_DEV_STATUS_ATTACHED,       /* Receive Subdev Topo Add Reply */
    IOTX_DM_DEV_STATUS_LOGINED,        /* Receive Subdev Login Reply */
    IOTX_DM_DEV_STATUS_ONLINE          /* After All Topic Subscribed */
} iotx_dm_dev_status_t;

typedef enum {
    DM_TSL_SERVICE_GET_FAILED = -13,
    DM_TSL_SERVICE_SET_FAILED = -12,
    DM_TSL_EVENT_GET_FAILED = -11,
    DM_TSL_EVENT_SET_FAILED = -10,
    DM_TSL_MEASUREPOINT_GET_FAILED = -9,
    DM_TSL_MEASUREPOINT_SET_FAILED = -8,
    DM_TSL_EVENT_NOT_EXIST = -7,
    DM_TSL_MEASUREPOINT_NOT_EXIST = -6,
    DM_TSL_SERVICE_NOT_EXIST = -5,
    DM_JSON_PARSE_FAILED = -4,
} dm_error_code_t;

#define IOTX_DM_POST_MEASUREPOINT_ALL (NULL)

int iotx_dm_open(void);
int iotx_dm_connect(_IN_ iotx_dm_init_params_t *init_params);
int iotx_dm_subscribe(_IN_ int devid);
int iotx_dm_close(void);
int iotx_dm_yield(int timeout_ms);
void iotx_dm_dispatch(void);

int iotx_dm_post_rawdata(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len);

int iotx_dm_set_opt(int opt, void *data);
int iotx_dm_get_opt(int opt, void *data);
int iotx_dm_subscribe_user_topic(char *topic, void *user_callback);

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
int iotx_dm_upstream_common(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len, _IN_ char *method);

/*
int iotx_dm_post_measurepoint(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len);
int iotx_dm_query_attribute(_IN_ int devid, _IN_ char* payload, _IN_ int payload_len);
 */
#ifdef DEVICE_MEASUREPOINT_RESUME
int iotx_dm_resume_measurepoint(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len);
#endif /* #ifdef DEVICE_MEASUREPOINT_RESUME */
int iotx_dm_post_event(_IN_ int devid, _IN_ char *identifier, _IN_ int identifier_len, _IN_ char *payload,
                       _IN_ int payload_len);

int iotx_dm_send_measurepoint_set_response(_IN_ int devid, _IN_ char *msgid, _IN_ int msgid_len, _IN_ iotx_dm_error_code_t code,
                                   _IN_ char *payload, _IN_ int payload_len, void *ctx);

int iotx_dm_send_service_invoke_response(_IN_ int devid, _IN_ char *msgid, _IN_ int msgid_len, _IN_ iotx_dm_error_code_t code,
                                  _IN_ char *identifier, _IN_ int identifier_len,
                                  _IN_ char *payload, _IN_ int payload_len, void *ctx);
#endif

#ifdef DEVICE_MODEL_GATEWAY
int iotx_dm_topo_get(void);
int iotx_dm_subdev_query(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                         _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                         _OU_ int *devid);
int iotx_dm_subdev_create(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                          _IN_ char product_secret[IOTX_PRODUCT_SECRET_LEN + 1],
                          _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                          _IN_ char device_secret[IOTX_DEVICE_SECRET_LEN + 1], _OU_ int *devid);
int iotx_dm_subdev_destroy(_IN_ int devid);
int iotx_dm_subdev_number(void);
int iotx_dm_subdev_register(_IN_ int devid);
int iotx_dm_subdev_topo_add(_IN_ int devid);
int iotx_dm_subdev_topo_del(_IN_ int devid);
int iotx_dm_subdev_login(_IN_ int devid);
int iotx_dm_subdev_login_batch(_IN_ int devid, int* sub_devids, int sub_devids_len);
int iotx_dm_subdev_logout(_IN_ int devid);
int iotx_dm_get_device_type(_IN_ int devid, _OU_ int *type);
int iotx_dm_get_device_avail_status(_IN_ int devid, _OU_ iotx_dm_dev_avail_t *status);
int iotx_dm_get_device_status(_IN_ int devid, _OU_ iotx_dm_dev_status_t *status);
#ifdef DEVICE_MODEL_SUBDEV_OTA
    int iotx_dm_send_firmware_version(int devid, const char *firmware_version);
    int iotx_dm_ota_switch_device(_IN_ int devid);
#endif /* DEVICE_MODEL_SUBDEV_OTA */
#endif /* DEVICE_MODEL_GATEWAY */
#endif /* _IOT_EXPORT_DM_H_ */
