#ifndef _INFRA_COMPAT_H_
#define _INFRA_COMPAT_H_

#include "infra_defs.h"

#if defined(__cplusplus)
extern "C" {
#endif

#undef  being_deprecated
#define being_deprecated

typedef enum _IOT_LogLevel {
    IOT_LOG_NONE = 0,
    IOT_LOG_CRIT,
    IOT_LOG_ERROR,
    IOT_LOG_WARNING,
    IOT_LOG_INFO,
    IOT_LOG_DEBUG,
} IOT_LogLevel;

void IOT_SetLogLevel(IOT_LogLevel level);
void IOT_DumpMemoryStats(IOT_LogLevel level);

/**
 * @brief event list used for iotx_regist_event_monitor_cb
 */
enum iotx_event_t {
    IOTX_CONN_CLOUD = 0x2000,       /* Device try to connect cloud */
    IOTX_CONN_CLOUD_FAIL,           /* Device fails to connect cloud, refer to net_sockets.h for error code */
    IOTX_CONN_CLOUD_SUC,            /* Device connects cloud successfully */
    IOTX_RESET = 0x3000,            /* EnOS reset success (just got reset response from cloud without any other operation) */
};

/**
 * @brief register callback to monitor all event from system.
 *
 * @param callback, when some event occurs, the system will trigger callback to user.
 *                  refer to enum iotx_event_t for event list supported.
 *
 * @return 0 when success, -1 when fail.
 * @note: user should make sure that callback is not block and runs to complete fast.
 */
int iotx_event_regist_cb(void (*monitor_cb)(int event));

/**
 * @brief post event to trigger callback resitered by iotx_event_regist_cb
 *
 * @param event, event id, refer to iotx_event_t
 *
 * @return 0 when success, -1 when fail.
 */
int iotx_event_post(int event);

typedef enum {
    ITE_CONNECT_SUCC,
    ITE_CONNECT_FAIL,
    ITE_DISCONNECTED,
    ITE_RAWDATA_ARRIVED,
    ITE_SERVICE_REQUEST,
    ITE_MEASUREPOINT_SET,
    ITE_MEASUREPOINT_GET,
/*    ITE_REPORT_REPLY, */
    ITE_THING_REPLY,
/*    ITE_TIMESTAMP_REPLY,*/
    ITE_TOPO_GET_REPLY,
/*    ITE_PERMIT_JOIN,*/
    ITE_DEVICE_REGISTER_REPLY,
    ITE_COMBINE_DISABLE,
    ITE_COMBINE_ENABLE,
    ITE_COMBINE_DELETE,
    ITE_INITIALIZE_COMPLETED,
    ITE_MQTT_CONNECT_SUCC,
    ITE_CLOUD_ERROR,
    ITE_DYNAMIC_ACTIVATE_DEVICE_SECRET,
    ITE_IDENTITY_RESPONSE,
    ITE_STATE_EVERYTHING,
    ITE_STATE_USER_INPUT,
    ITE_STATE_SYS_DEPEND,
    ITE_STATE_MQTT_COMM,
    ITE_STATE_WIFI_PROV,
    ITE_STATE_COAP_LOCAL,
    ITE_STATE_HTTP_COMM,
    ITE_STATE_OTA,
    ITE_STATE_DEV_BIND,
    ITE_STATE_DEV_MODEL
} iotx_ioctl_event_t;

#define IOT_RegisterCallback(evt, cb)           iotx_register_for_##evt(cb)
#define DECLARE_EVENT_CALLBACK(evt, cb)         int iotx_register_for_##evt(cb);
#define DEFINE_EVENT_CALLBACK(evt, cb)          int iotx_register_for_##evt(cb) { \
        if (evt < 0 || evt >= sizeof(g_impl_event_map)/sizeof(impl_event_map_t)) {return -1;} \
        g_impl_event_map[evt].callback = (void *)callback;return 0;}

DECLARE_EVENT_CALLBACK(ITE_CONNECT_SUCC,         int (*cb)(void))
DECLARE_EVENT_CALLBACK(ITE_CONNECT_FAIL,         int (*cb)(void))
DECLARE_EVENT_CALLBACK(ITE_DISCONNECTED,         int (*cb)(void))
DECLARE_EVENT_CALLBACK(ITE_RAWDATA_ARRIVED,      int (*cb)(const int, const unsigned char *, const int))
/* DECLARE_EVENT_CALLBACK(ITE_SERVICE_REQUEST,      int (*cb)(const int, const char *, const int, const char *, const int, char **, int *)) */
DECLARE_EVENT_CALLBACK(ITE_SERVICE_REQUEST,      int (*cb)(int, const char *, int, const char *, int, void *))
DECLARE_EVENT_CALLBACK(ITE_MEASUREPOINT_SET,     int (*cb)(const int, const char *, const int, void*))
DECLARE_EVENT_CALLBACK(ITE_MEASUREPOINT_GET,     int (*cb)(const int, const char *, const int, char **, int *))
/* DECLARE_EVENT_CALLBACK(ITE_REPORT_REPLY,         int (*cb)(const int, const int, const int, const char *, const int, const char *, const int)) */
DECLARE_EVENT_CALLBACK(ITE_THING_REPLY,          int (*cb)(const int, const int, const int, const char *, const int, const char *, const int))
/*DECLARE_EVENT_CALLBACK(ITE_TIMESTAMP_REPLY,      int (*cb)(const char *))*/
DECLARE_EVENT_CALLBACK(ITE_TOPO_GET_REPLY,       int (*cb)(const int, const int, const int, const char *, const int))
DECLARE_EVENT_CALLBACK(ITE_DEVICE_REGISTER_REPLY,int (*cb)(const int, const int, const int))
DECLARE_EVENT_CALLBACK(ITE_COMBINE_DISABLE,      int (*cb)(const int))
DECLARE_EVENT_CALLBACK(ITE_COMBINE_ENABLE,       int (*cb)(const int))
DECLARE_EVENT_CALLBACK(ITE_COMBINE_DELETE,       int (*cb)(const int, const char*, const int, const char*, const int))
/*DECLARE_EVENT_CALLBACK(ITE_PERMIT_JOIN,          int (*cb)(const char *, const int))*/
DECLARE_EVENT_CALLBACK(ITE_INITIALIZE_COMPLETED, int (*cb)(const int))
DECLARE_EVENT_CALLBACK(ITE_MQTT_CONNECT_SUCC,    int (*cb)(void))
DECLARE_EVENT_CALLBACK(ITE_CLOUD_ERROR,          int (*cb)(const int, const char *, const char *))
DECLARE_EVENT_CALLBACK(ITE_DYNAMIC_ACTIVATE_DEVICE_SECRET, int (*cb)(const char *))
DECLARE_EVENT_CALLBACK(ITE_IDENTITY_RESPONSE,    int (*cb)(const char *))

typedef int (*state_handler_t)(const int state_code, const char *state_message);
DECLARE_EVENT_CALLBACK(ITE_STATE_EVERYTHING, state_handler_t cb);
DECLARE_EVENT_CALLBACK(ITE_STATE_USER_INPUT, state_handler_t cb);
DECLARE_EVENT_CALLBACK(ITE_STATE_SYS_DEPEND, state_handler_t cb);
DECLARE_EVENT_CALLBACK(ITE_STATE_MQTT_COMM,  state_handler_t cb);
DECLARE_EVENT_CALLBACK(ITE_STATE_WIFI_PROV,  state_handler_t cb);
DECLARE_EVENT_CALLBACK(ITE_STATE_COAP_LOCAL, state_handler_t cb);
DECLARE_EVENT_CALLBACK(ITE_STATE_HTTP_COMM,  state_handler_t cb);
DECLARE_EVENT_CALLBACK(ITE_STATE_OTA,        state_handler_t cb);
DECLARE_EVENT_CALLBACK(ITE_STATE_DEV_BIND,   state_handler_t cb);
DECLARE_EVENT_CALLBACK(ITE_STATE_DEV_MODEL,  state_handler_t cb);

int iotx_state_event(const int event, const int state_code, const char *state_message_format, ...);

void *iotx_event_callback(int evt);

typedef struct {
    uint16_t        port;
    uint8_t         init;
    char            *host_name;
    char            *client_id;
    char            *username;
    char            *password;
    const char      *pub_key;
} iotx_conn_info_t, *iotx_conn_info_pt;

int IOT_SetupConnInfo(const char *product_key,
                      const char *device_key,
                      const char *device_secret,
                      void **info_ptr);


typedef enum {
    IOTX_IOCTL_SET_REGION,              /* value(int*): iotx_cloud_region_types_t */
    IOTX_IOCTL_GET_REGION,              /* value(int*) */
    IOTX_IOCTL_SET_MQTT_DOMAIN,         /* value(const char*): point to mqtt domain string */
    IOTX_IOCTL_SET_DYNAMIC_ACTIVATE,    /* value(int*): 0 - Disable Dynamic Activate, 1 - Enable Dynamic Activate */
    IOTX_IOCTL_GET_DYNAMIC_ACTIVATE,    /* value(int*) */
    IOTX_IOCTL_RECV_THING_REPLY,         /* value(int*): 0 - Disable thing reply by cloud; 1 - Enable thing reply by cloud */
    IOTX_IOCTL_SEND_MEASUREPOINT_SET_AUTO_REPLY,     /* value(int*): 0 - Disable send measurepoint set reply automatically by devid; 1 - Enable measurepoint set reply automatically by devid */
    IOTX_IOCTL_SET_SUBDEV_SIGN,         /* value(const char*): only for slave device, set signature of subdevice */
    IOTX_IOCTL_GET_SUBDEV_LOGIN,        /* value(int*): 0 - SubDev is logout; 1 - SubDev is login */
    IOTX_IOCTL_SET_OTA_DEV_ID,          /* value(int*):     select the device to do OTA according to devid */
    IOTX_IOCTL_SET_CUSTOMIZE_INFO,      /* value(char*): set mqtt clientID customize information */
    IOTX_IOCTL_SET_MQTT_PORT,           /* value(uint16_t *) modify mqtt server port number */
    
    IOTX_IOCTL_QUERY_DEVID,             /* value(iotx_dev_meta_info_t*): device meta info, only productKey and deviceKey is required, ret value is subdev_id or -1 */
    IOTX_IOCTL_SUB_USER_TOPIC,          /* subscribe a topic according to user topic and callback */
    IOTX_IOCTL_SET_PRODUCT_KEY,         /* vale(char *) - set product key */
    IOTX_IOCTL_GET_PRODUCT_KEY,         /* vale(char[IOTX_PRODUCT_KEY_LEN + 1]) - get product key */
    IOTX_IOCTL_SET_PRODUCT_SECRET,      /* vale(char *) - set product secret */
    IOTX_IOCTL_GET_PRODUCT_SECRET,      /* vale(char[IOTX_PRODUCT_SECRET_LEN + 1]) - get product secret */
    IOTX_IOCTL_SET_DEVICE_KEY,         /* vale(char *) - set device key */
    IOTX_IOCTL_GET_DEVICE_KEY,         /* vale(char[IOTX_DEVICE_KEY_LEN + 1]) - get device key */
    IOTX_IOCTL_SET_DEVICE_SECRET,       /* vale(char *) - set device secret */
    IOTX_IOCTL_GET_DEVICE_SECRET        /* vale(char[IOTX_DEVICE_SECRET_LEN + 1]) - get device secret */
} iotx_ioctl_option_t;

typedef enum {
    IMPL_ENOS_IOCTL_SWITCH_THING_REPLY,                  /* only for master device, choose whether you need receive thing reply message */
    IMPL_ENOS_IOCTL_SWITCH_MEASUREPOINT_SET_AUTO_REPLY,       /* only for master device, choose whether you need send measurepoint set reply message */
    IMPL_ENOS_IOCTL_MAX
} impl_enos_ioctl_cmd_t;

/**
 * @brief Setup Demain type, should be called before MQTT connection.
 *
 * @param [in] option: see iotx_ioctl_option_t.
 *
 * @return None.
 * @see None.
 */
int IOT_Ioctl(int option, void *data);


/* compatible for V2.3.0 */
#define IOTX_CLOUD_DOMAIN_SH        IOTX_CLOUD_REGION_SHANGHAI
#define IOTX_CLOUD_DOMAIN_SG        IOTX_CLOUD_REGION_SINGAPORE
#define IOTX_CLOUD_DOMAIN_JP        IOTX_CLOUD_REGION_JAPAN
#define IOTX_CLOUD_DOMAIN_US        IOTX_CLOUD_REGION_USA_WEST
#define IOTX_CLOUD_DOMAIN_GER       IOTX_CLOUD_REGION_GERMANY
#define IOTX_IOCTL_SET_DOMAIN       IOTX_IOCTL_SET_REGION
#define IOTX_IOCTL_GET_DOMAIN       IOTX_IOCTL_GET_REGION

#define IOT_OpenLog(arg)
#define IOT_CloseLog()              IOT_SetLogLevel(IOT_LOG_NONE)
#define IOT_LOG_EMERG               IOT_LOG_NONE

#define IOT_EnOS_Post               IOT_EnOS_Report
/* compatible for V2.3.0 end */

typedef enum {
    HAL_AES_ENCRYPTION = 0,
    HAL_AES_DECRYPTION = 1,
} AES_DIR_t;

#define AES_DECRYPTION HAL_AES_DECRYPTION
#define AES_ENCRYPTION HAL_AES_ENCRYPTION

typedef void *p_Aes128_t;

#define NETWORK_ADDR_LEN        (16)

typedef struct _network_addr_t {
    unsigned char
    addr[NETWORK_ADDR_LEN];
    unsigned short  port;
} NetworkAddr;

#if defined(__cplusplus)
}
#endif

#endif  /* _INFRA_COMPAT_H_ */
