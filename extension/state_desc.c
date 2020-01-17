#include <stdio.h>
#include "infra_state.h"

typedef struct code_desc_s {
    int     code;
    char   *desc;
} code_desc_t;

static code_desc_t user_input_desc[] = {
    {STATE_USER_INPUT_NULL_POINTER, "User input parameters contain unacceptable NULL pointer"},
    {STATE_USER_INPUT_OUT_RANGE, "Some user input parameter(s) has value out of acceptable range"},
    {STATE_USER_INPUT_PK, "User input parameters contain unacceptable productKey"},
    {STATE_USER_INPUT_PS, "User input parameters contain unacceptable productSecret"},
    {STATE_USER_INPUT_DN, "User input parameters contain unacceptable deviceName"},
    {STATE_USER_INPUT_DS, "User input parameters contain unacceptable deviceSecret"},
    {STATE_USER_INPUT_HTTP_DOMAIN, "User input parameters contain unacceptable HTTP domain name"},
    {STATE_USER_INPUT_MQTT_DOMAIN, "User input parameters contain unacceptable MQTT domain name"},
    {STATE_USER_INPUT_HTTP_PORT, "User input parameters contain unacceptable HTTP port"},
    {STATE_USER_INPUT_HTTP_TIMEOUT, "User input parameters contain unacceptable HTTP timeout"},
    {STATE_USER_INPUT_HTTP_OPTION, "User input parameters contain unacceptable HTTP options"},
    {STATE_USER_INPUT_HTTP_POST_DATA, "User input parameters contain unacceptable post data"},
    {STATE_USER_INPUT_HTTP_URL, "User input parameters contain unacceptable HTTP URL"},
    {STATE_USER_INPUT_HTTP_PATH, "User input parameters contain unacceptable HTTP path"},
    {STATE_USER_INPUT_META_INFO, "User input parameters contain unacceptable PK/PS/DN/DS"},
    {STATE_USER_INPUT_DEVID, "User input parameters contain unacceptable deviceID"},
    {STATE_USER_INPUT_DEVICE_TYPE, "User input parameters contain unacceptable device type"},
    {STATE_USER_INPUT_MSG_TYPE, "User input parameters contain unacceptable message type"},
    {STATE_USER_INPUT_INVALID, "User input parameters contain unacceptable value"},
    {0, "Should not reach"}
};

static code_desc_t sys_depend_desc[] = {
    {STATE_SYS_DEPEND_MALLOC, "SDK run into exception when invoking HAL_Malloc()"},
    {STATE_SYS_DEPEND_KV_GET, "SDK run into exception when invoking HAL_Kv_Get()"},
    {STATE_SYS_DEPEND_KV_SET, "SDK run into exception when invoking HAL_Kv_Set()"},
    {STATE_SYS_DEPEND_KV_DELETE, "SDK run into exception when invoking HAL_Kv_Del()"},
    {STATE_SYS_DEPEND_MUTEX_CREATE, "SDK run into exception when invoking HAL_MutexCreate()"},
    {STATE_SYS_DEPEND_MUTEX_LOCK, "SDK run into exception when invoking HAL_MutexLock()"},
    {STATE_SYS_DEPEND_MUTEX_UNLOCK, "SDK run into exception when invoking HAL_MutexUnlock()"},
    {STATE_SYS_DEPEND_NWK_CLOSE, "SDK run into exception when TX or RX through lower network layer"},
    {STATE_SYS_DEPEND_NWK_TIMEOUT, "SDK run into timeout when TX or RX through lower network layer"},
    {STATE_SYS_DEPEND_NWK_INVALID_HANDLE, "SDK run into invalid handler when lookup network lower layer connection"},
    {STATE_SYS_DEPEND_NWK_READ_ERROR, "SDK run into exception when RX through lower network layer"},
    {STATE_SYS_DEPEND_SEMAPHORE_CREATE, "SDK run into exception when invoking HAL_SemaphoreCreate()"},
    {STATE_SYS_DEPEND_SEMAPHORE_WAIT, "SDK run into exception when invoking HAL_SemaphoreWait()"},
    {STATE_SYS_DEPEND_SNPRINTF, "SDK run into exception when invoking HAL_Snprintf()"},
    {STATE_SYS_DEPEND_FIRMWAIRE_WIRTE, "SDK run into exception when invoking HAL_Firmware_Persistence_Write()"},
    {0, "Should not reach"}
};

static code_desc_t mqtt_desc[] = {
    {STATE_MQTT_CONNACK_VERSION_UNACCEPT, "Deserialized CONNACK from MQTT server says protocol version is unacceptable"},
    {STATE_MQTT_CONNACK_IDENT_REJECT, "Deserialized CONNACK from MQTT server says identifier is rejected"},
    {STATE_MQTT_CONNACK_SERVICE_NA, "Deserialized CONNACK from MQTT server says service is not available"},
    {STATE_MQTT_CONNACK_NOT_AUTHORIZED, "Deserialized CONNACK from MQTT server says it failed to authorize"},
    {STATE_MQTT_CONNACK_BAD_USERDATA, "Deserialized CONNACK from MQTT server says username/password is invalid"},
    {STATE_MQTT_WAIT_RECONN_TIMER, "Skip current reconnect attemption until next timer expiration"},
    {STATE_MQTT_SIGN_HOSTNAME_BUF_SHORT, "Reserved buffer is too short when generate device signature for hostname"},
    {STATE_MQTT_SIGN_USERNAME_BUF_SHORT, "Reserved buffer is too short when generate device signature for username"},
    {STATE_MQTT_SIGN_CLIENTID_BUF_SHORT, "Reserved buffer is too short when generate device signature for clientId"},
    {STATE_MQTT_SIGN_SOURCE_BUF_SHORT, "Reserved buffer is too short for signature generate source"},
    {STATE_MQTT_WRAPPER_INIT_FAIL, "SDK run into exception when invoking lower layer wrapper_mqtt_init()"},
    {STATE_MQTT_SERIALIZE_CONN_ERROR, "Failed to serialize connect request"},
    {STATE_MQTT_SERIALIZE_PUBACK_ERROR, "Failed to serialize acknowledge message of publish"},
    {STATE_MQTT_SERIALIZE_PINGREQ_ERROR, "Failed to serialize ping request"},
    {STATE_MQTT_SERIALIZE_SUB_ERROR, "Failed to serialize subscribe request"},
    {STATE_MQTT_SERIALIZE_UNSUB_ERROR, "Failed to serialize unsubscribe request"},
    {STATE_MQTT_SERIALIZE_PUB_ERROR, "Failed to serialize publish message"},
    {STATE_MQTT_DESERIALIZE_CONNACK_ERROR, "Failed to deserialize connect response"},
    {STATE_MQTT_DESERIALIZE_SUBACK_ERROR, "Failed to deserialize subscribe response"},
    {STATE_MQTT_DESERIALIZE_PUB_ERROR, "Failed to deserialize publish response"},
    {STATE_MQTT_DESERIALIZE_UNSUBACK_ERROR, "Failed to deserialize unsubscribe response"},
    {STATE_MQTT_PACKET_READ_ERROR, "Failed to read MQTT packet from network"},
    {STATE_MQTT_CONNACK_UNKNOWN_ERROR, "Failed to interpret CONNACK from MQTT server"},
    {STATE_MQTT_RX_BUFFER_TOO_SHORT, "Reserved buffer is too short when retrieve content from network"},
    {STATE_MQTT_TX_BUFFER_TOO_SHORT, "Reserved buffer is too short when compose content going to network"},
    {STATE_MQTT_TOPIC_BUF_TOO_SHORT, "Reserved buffer is too short when compose topic for MQTT outgoing message"},
    {STATE_MQTT_CONN_RETRY_EXCEED_MAX, "Retried time exceeds maximum when perform IOT_MQTT_Construct()"},
    {STATE_MQTT_QOS1_REPUB_EXCEED_MAX, "Re-publish QoS1 message exceeds maximum"},
    {STATE_MQTT_PUB_QOS_INVALID, "Invalid QoS input when publish MQTT message"},
    {STATE_MQTT_IN_OFFLINE_STATUS, "Skip current action because MQTT connection is break"},
    {STATE_MQTT_RECV_UNKNOWN_PACKET, "Receive MQTT packet from network can not be interpret"},
    {STATE_MQTT_CLI_EXCEED_MAX, "MQTT connection instance exceed allowed maximum"},
    {STATE_MQTT_SUB_EXCEED_MAX, "MQTT subscribe instance exceed allowed maximum"},
    {STATE_MQTT_UNEXP_TOPIC_FORMAT, "Invalid MQTT topic can not be interpret"},
    {STATE_MQTT_SYNC_SUB_TIMEOUT, "Wait too long time after syncronized subscribe request sent"},
    {STATE_MQTT_ASYNC_STACK_CONN_IN_PROG, "Last connect request has not been responsed in async protocol stack"},
    {STATE_MQTT_ASYNC_STACK_NOT_SUPPORT, "Currrent SDK configuration does not support working with async protocol stack"},
    {STATE_MQTT_ASYNC_STACK_UNKNOWN_EVENT, "Got unknown event when work with async protocol stack"},
    {STATE_MQTT_CONN_USER_INFO, "Report connect relative parameters such as username and password"},
    {STATE_MQTT_SUB_INFO, "Report subscribe relative parameters such as topic string"},
    {STATE_MQTT_PUB_INFO, "Report publish relative parameters such as topic string"},
    {0, "Should not reach"}
};

static code_desc_t coap_desc[] = {
    {0, "Should not reach"}
};

static code_desc_t http_desc[] = {
    {0, "Should not reach"}
};

static code_desc_t ota_desc[] = {
    {0, "Should not reach"}
};

static code_desc_t dev_model_desc[] = {
    {STATE_DEV_MODEL_MASTER_ALREADY_OPEN, "Master device already open so skip current open action"},
    {STATE_DEV_MODEL_MASTER_ALREADY_CONNECT, "Master device already connect so skip current connect action"},
    {STATE_DEV_MODEL_MASTER_NOT_OPEN_YET, "Master device not open yet so skip current action"},
    {STATE_DEV_MODEL_MASTER_NOT_CONNECT_YET, "Master device not connect yet so skip current action"},
    {STATE_DEV_MODEL_DEVICE_ALREADY_EXIST, "Requested device already created"},
    {STATE_DEV_MODEL_DEVICE_NOT_FOUND, "Requested device not found in list"},
    {STATE_DEV_MODEL_SUBD_NOT_DELETEABLE, "Requested device can not be deleted"},
    {STATE_DEV_MODEL_SUBD_NOT_LOGIN, "Requested device not in login status so can not logout"},
    {STATE_DEV_MODEL_INTERNAL_ERROR, "Internal error happens in device model function"},
    {STATE_DEV_MODEL_INTERNAL_MQTT_DUP_INIT, "Internal event about MQTT connect happens in device model function"},
    {STATE_DEV_MODEL_INTERNAL_MQTT_NOT_INIT_YET, "Internal error about MQTT unconnect happens in device model function"},
    {STATE_DEV_MODEL_CM_OPEN_FAILED, "Failed to open handler for cloud abstract layer"},
    {STATE_DEV_MODEL_CM_FD_NOT_FOUND, "Failed to find file descriptor in cloud abstract layer"},
    {STATE_DEV_MODEL_CM_FD_ERROR, "Internal error about file descriptor happens in device model function"},
    {STATE_DEV_MODEL_MQTT_CONNECT_FAILED, "Failed to connect MQTT in device model function"},
    {STATE_DEV_MODEL_RECV_UNEXP_MQTT_PUB, "Got unexpected MQTT message from server in device model"},
    {STATE_DEV_MODEL_WRONG_JSON_FORMAT, "Got MQTT message from server but its JSON format is wrong"},
    {STATE_DEV_MODEL_GET_JSON_ITEM_FAILED, "Failed to lookup value for specified key when parse JSON"},
    {STATE_DEV_MODEL_SERVICE_CTX_NOT_EXIST, "Service respond does not have correct request context in device model"},
    {STATE_DEV_MODEL_OTA_NOT_ENABLED, "OTA service is not enabled in device model"},
    {STATE_DEV_MODEL_OTA_NOT_INITED, "OTA service is not initialized correctly in device model"},
    {STATE_DEV_MODEL_OTA_INIT_FAILED, "OTA service is initialized but failed in device model"},
    {STATE_DEV_MODEL_OTA_STILL_IN_PROGRESS, "OTA for some sub-device is not finished yet so skip device switching"},
    {STATE_DEV_MODEL_OTA_IMAGE_CHECK_FAILED, "OTA firmware downloaded failed to pass integrity check"},
    {STATE_DEV_MODEL_OTA_TYPE_ERROR, "OTA type is neither cota or fota"},
    {STATE_DEV_MODEL_OTA_FETCH_FAILED, "OTA fetching from cloud failed to get expected content"},
    {STATE_DEV_MODEL_GATEWAY_NOT_ENABLED, "Gateway/Sub-device management function is not configured on"},
    {STATE_DEV_MODEL_IN_RAWDATA_SOLO, "Configured rawdata+solo mode, will not proceed JSON messages"},
    {STATE_DEV_MODEL_DUP_UPSTREAM_MSG, "Duplicated upstream device model messages"},
    {STATE_DEV_MODEL_UPSTREAM_REC_NOT_FOUND, "Corresponding upstream record not found for downstream messages"},
    {STATE_DEV_MODEL_REFUSED_BY_CLOUD, "Got negative respond from cloud in device model"},
    {STATE_DEV_MODEL_INVALID_DM_OPTION, "Encount unexpected option when invoke dm_opt_get()"},
    {STATE_DEV_MODEL_URL_SPLIT_FAILED, "Encount parsing failure when process URL in respond from cloud"},
    {STATE_DEV_MODEL_ENOS_MSG_PARSE_FAILED, "Failed to parse EnOS request or respond messages"},
    {STATE_DEV_MODEL_LOG_REPORT_ERROR, "Failed to upload device log to cloud"},
    {STATE_DEV_MODEL_IN_AUTOSUB_MODE, "Bypass current topic subscription since auto subscribed"},
    {STATE_DEV_MODEL_RX_CLOUD_MESSAGE, "Got message from cloud in device model"},
    {STATE_DEV_MODEL_TX_CLOUD_MESSAGE, "Sending message to cloud in device model"},
    {STATE_DEV_MODLE_RX_LOCAL_MESSAGE, "Got message from local in device model"},
    {STATE_DEV_MODLE_TX_LOCAL_MESSAGE, "Sending message to local in device model"},
    {STATE_DEV_MODEL_MSGQ_OPERATION, "Processing message queue of device model and external modules"},
    {STATE_DEV_MODEL_MSGQ_FULL, "Message queue in device model is full"},
    {STATE_DEV_MODEL_MSGQ_EMPTY, "Message queue in device model is empty"},
    {STATE_DEV_MODEL_CTX_LIST_INSERT, "Cache list in device model get inserted new element"},
    {STATE_DEV_MODEL_CTX_LIST_REMOVE, "Cache list in device model get removed element"},
    {STATE_DEV_MODEL_CTX_LIST_FADEOUT, "Element in cache list of device model removed since life-time over"},
    {STATE_DEV_MODEL_CTX_LIST_FULL, "Cache list in device model get filled full"},
    {STATE_DEV_MODEL_CTX_LIST_EMPTY, "Cache list in device model get consumed empty"},
    {STATE_DEV_MODLE_LOG_REPORT_STOP, "Log post to cloud get stopped"},
    {STATE_DEV_MODLE_LOG_REPORT_SWITCH, "Log post to cloud get switched"},
    {STATE_DEV_MODLE_LOG_REPORT_SEND, "Log post to cloud get transmitted"},
    {STATE_DEV_MODEL_SYNC_REQ_LIST, "Operating to list of sync request in device model"},
    {STATE_DEV_MODEL_ENOS_PROT_EVENT, "Event of EnOS protocol processing is happenning"},
    {STATE_DEV_MODEL_INVALID_ENOS_PAYLOAD, "Got invalid EnOS protocol mqtt packet"},
    {STATE_DEV_MODEL_INVALID_ENOS_TOPIC, "Got invalid requested EnOS protocol mqtt topic"},
    {STATE_DEV_MODEL_YIELD_STOPPED, "IOT_EnOS_Yield already stopped"},
    {STATE_DEV_MODEL_YIELD_RUNNING, "IOT_EnOS_Yield still running"},
    {0, "Should not reach"}
};

static code_desc_t *decs_tbl[] = {
    user_input_desc,
    sys_depend_desc,
    mqtt_desc,
    coap_desc,
    http_desc,
    ota_desc,
    dev_model_desc,
    NULL
};

const char *IOT_Extension_StateDesc(const int code)
{
    code_desc_t    *category = NULL;
    int             cate_idx = 0;
    int             code_idx = 0;
    int             cate_num = (int)(sizeof(decs_tbl) / sizeof(code_desc_t *)) - 1;

    if (STATE_SUCCESS == code) {
        return "Success";
    }

    cate_idx = (((-code) & (0xFF << 8)) >> 8) - 1;
    code_idx = ((-code) & (0xFF)) - 1;

    if (cate_idx < cate_num) {
        category = decs_tbl[cate_idx];
        if (code == category[code_idx].code) {
            return category[code_idx].desc;
        }
    }

    return "N/A - Description not found";
}
