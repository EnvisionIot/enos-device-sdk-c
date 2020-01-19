/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */

#ifndef __INFRA_STATE_H__
#define __INFRA_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define STATE_BASE                                  (0x0000)

/* General: 0x0000 ~ 0x00FF */
/* API works as expected and returns sucess */
#define STATE_SUCCESS                               (STATE_BASE - 0x0000)

/* General: 0x0000 ~ 0x00FF */

/* User Input: 0x0100 ~ 0x01FF */
#define STATE_USER_INPUT_BASE                       (-0x0100)

/* User input parameters contain unacceptable NULL pointer */
#define STATE_USER_INPUT_NULL_POINTER               (STATE_USER_INPUT_BASE - 0x0001)
/* Some user input parameter(s) has value out of acceptable range */
#define STATE_USER_INPUT_OUT_RANGE                  (STATE_USER_INPUT_BASE - 0x0002)
/* User input parameters contain unacceptable productKey */
#define STATE_USER_INPUT_PK                         (STATE_USER_INPUT_BASE - 0x0003)
/* User input parameters contain unacceptable productSecret */
#define STATE_USER_INPUT_PS                         (STATE_USER_INPUT_BASE - 0x0004)
/* User input parameters contain unacceptable deviceKey */
#define STATE_USER_INPUT_DN                         (STATE_USER_INPUT_BASE - 0x0005)
/* User input parameters contain unacceptable deviceSecret */
#define STATE_USER_INPUT_DS                         (STATE_USER_INPUT_BASE - 0x0006)
/* User input parameters contain unacceptable HTTP domain name */
#define STATE_USER_INPUT_HTTP_DOMAIN                (STATE_USER_INPUT_BASE - 0x0007)
/* User input parameters contain unacceptable MQTT domain name */
#define STATE_USER_INPUT_MQTT_DOMAIN                (STATE_USER_INPUT_BASE - 0x0008)
/* User input parameters contain unacceptable HTTP port */
#define STATE_USER_INPUT_HTTP_PORT                  (STATE_USER_INPUT_BASE - 0x0009)
/* User input parameters contain unacceptable HTTP timeout */
#define STATE_USER_INPUT_HTTP_TIMEOUT               (STATE_USER_INPUT_BASE - 0x000A)
/* User input parameters contain unacceptable HTTP options */
#define STATE_USER_INPUT_HTTP_OPTION                (STATE_USER_INPUT_BASE - 0x000B)
/* User input parameters contain unacceptable post data */
#define STATE_USER_INPUT_HTTP_POST_DATA             (STATE_USER_INPUT_BASE - 0x000C)
/* User input parameters contain unacceptable HTTP URL */
#define STATE_USER_INPUT_HTTP_URL                   (STATE_USER_INPUT_BASE - 0x000D)
/* User input parameters contain unacceptable HTTP path */
#define STATE_USER_INPUT_HTTP_PATH                  (STATE_USER_INPUT_BASE - 0x000E)
/* User input parameters contain unacceptable PK/PS/DN/DS */
#define STATE_USER_INPUT_META_INFO                  (STATE_USER_INPUT_BASE - 0x000F)
/* User input parameters contain unacceptable deviceID */
#define STATE_USER_INPUT_DEVID                      (STATE_USER_INPUT_BASE - 0x0010)
/* User input parameters contain unacceptable device type */
#define STATE_USER_INPUT_DEVICE_TYPE                (STATE_USER_INPUT_BASE - 0x0011)
/* User input parameters contain unacceptable message type */
#define STATE_USER_INPUT_MSG_TYPE                   (STATE_USER_INPUT_BASE - 0x0012)
/* User input parameters contain unacceptable value */
#define STATE_USER_INPUT_INVALID                    (STATE_USER_INPUT_BASE - 0x0013)
/* User Input: 0x0100 ~ 0x01FF */

/* System: 0x0200 ~ 0x02FF */
#define STATE_SYS_DEPEND_BASE                       (-0x0200)

/* SDK run into exception when invoking HAL_Malloc() */
#define STATE_SYS_DEPEND_MALLOC                     (STATE_SYS_DEPEND_BASE - 0x0001)
/* SDK run into exception when invoking HAL_Kv_Get() */
#define STATE_SYS_DEPEND_KV_GET                     (STATE_SYS_DEPEND_BASE - 0x0002)
/* SDK run into exception when invoking HAL_Kv_Set() */
#define STATE_SYS_DEPEND_KV_SET                     (STATE_SYS_DEPEND_BASE - 0x0003)
/* SDK run into exception when invoking HAL_Kv_Del() */
#define STATE_SYS_DEPEND_KV_DELETE                  (STATE_SYS_DEPEND_BASE - 0x0004)
/* SDK run into exception when invoking HAL_MutexCreate() */
#define STATE_SYS_DEPEND_MUTEX_CREATE               (STATE_SYS_DEPEND_BASE - 0x0005)
/* SDK run into exception when invoking HAL_MutexLock() */
#define STATE_SYS_DEPEND_MUTEX_LOCK                 (STATE_SYS_DEPEND_BASE - 0x0006)
/* SDK run into exception when invoking HAL_MutexUnlock() */
#define STATE_SYS_DEPEND_MUTEX_UNLOCK               (STATE_SYS_DEPEND_BASE - 0x0007)
/* SDK run into exception when TX or RX through lower network layer */
#define STATE_SYS_DEPEND_NWK_CLOSE                  (STATE_SYS_DEPEND_BASE - 0x0008)
/* SDK run into timeout when TX or RX through lower network layer */
#define STATE_SYS_DEPEND_NWK_TIMEOUT                (STATE_SYS_DEPEND_BASE - 0x0009)
/* SDK run into invalid handler when lookup network lower layer connection */
#define STATE_SYS_DEPEND_NWK_INVALID_HANDLE         (STATE_SYS_DEPEND_BASE - 0x000A)
/* SDK run into exception when RX through lower network layer */
#define STATE_SYS_DEPEND_NWK_READ_ERROR             (STATE_SYS_DEPEND_BASE - 0x000B)
/* SDK run into exception when invoking HAL_SemaphoreCreate() */
#define STATE_SYS_DEPEND_SEMAPHORE_CREATE           (STATE_SYS_DEPEND_BASE - 0x000C)
/* SDK run into exception when invoking HAL_SemaphoreWait() */
#define STATE_SYS_DEPEND_SEMAPHORE_WAIT             (STATE_SYS_DEPEND_BASE - 0x000D)
/* SDK run into exception when invoking HAL_Snprintf() */
#define STATE_SYS_DEPEND_SNPRINTF                   (STATE_SYS_DEPEND_BASE - 0x000E)
/* SDK run into exception when invoking HAL_Firmware_Persistence_Write() */
#define STATE_SYS_DEPEND_FIRMWAIRE_WIRTE            (STATE_SYS_DEPEND_BASE - 0x000F)

/* System: 0x0200 ~ 0x02FF */

/* MQTT: 0x0300 ~ 0x03FF */
#define STATE_MQTT_BASE                             (-0x0300)

/* Deserialized CONNACK from MQTT server says protocol version is unacceptable */
#define STATE_MQTT_CONNACK_VERSION_UNACCEPT         (STATE_MQTT_BASE - 0x0001)
/* Deserialized CONNACK from MQTT server says identifier is rejected */
#define STATE_MQTT_CONNACK_IDENT_REJECT             (STATE_MQTT_BASE - 0x0002)
/* Deserialized CONNACK from MQTT server says service is not available */
#define STATE_MQTT_CONNACK_SERVICE_NA               (STATE_MQTT_BASE - 0x0003)
/* Deserialized CONNACK from MQTT server says it failed to authorize */
#define STATE_MQTT_CONNACK_NOT_AUTHORIZED           (STATE_MQTT_BASE - 0x0004)
/* Deserialized CONNACK from MQTT server says username/password is invalid */
#define STATE_MQTT_CONNACK_BAD_USERDATA             (STATE_MQTT_BASE - 0x0005)
/* Skip current reconnect attemption until next timer expiration */
#define STATE_MQTT_WAIT_RECONN_TIMER                (STATE_MQTT_BASE - 0x0006)
/* Reserved buffer is too short when generate device signature for hostname */
#define STATE_MQTT_SIGN_HOSTNAME_BUF_SHORT          (STATE_MQTT_BASE - 0x0007)
/* Reserved buffer is too short when generate device signature for username */
#define STATE_MQTT_SIGN_USERNAME_BUF_SHORT          (STATE_MQTT_BASE - 0x0008)
/* Reserved buffer is too short when generate device signature for clientId */
#define STATE_MQTT_SIGN_CLIENTID_BUF_SHORT          (STATE_MQTT_BASE - 0x0009)
/* Reserved buffer is too short for signature generate source */
#define STATE_MQTT_SIGN_SOURCE_BUF_SHORT            (STATE_MQTT_BASE - 0x000A)
/* SDK run into exception when invoking lower layer wrapper_mqtt_init() */
#define STATE_MQTT_WRAPPER_INIT_FAIL                (STATE_MQTT_BASE - 0x000B)
/* Failed to serialize connect request */
#define STATE_MQTT_SERIALIZE_CONN_ERROR             (STATE_MQTT_BASE - 0x000C)
/* Failed to serialize acknowledge message of publish */
#define STATE_MQTT_SERIALIZE_PUBACK_ERROR           (STATE_MQTT_BASE - 0x000D)
/* Failed to serialize ping request */
#define STATE_MQTT_SERIALIZE_PINGREQ_ERROR          (STATE_MQTT_BASE - 0x000E)
/* Failed to serialize subscribe request */
#define STATE_MQTT_SERIALIZE_SUB_ERROR              (STATE_MQTT_BASE - 0x000F)
/* Failed to serialize unsubscribe request */
#define STATE_MQTT_SERIALIZE_UNSUB_ERROR            (STATE_MQTT_BASE - 0x0010)
/* Failed to serialize publish message */
#define STATE_MQTT_SERIALIZE_PUB_ERROR              (STATE_MQTT_BASE - 0x0011)
/* Failed to deserialize connect response */
#define STATE_MQTT_DESERIALIZE_CONNACK_ERROR        (STATE_MQTT_BASE - 0x0012)
/* Failed to deserialize subscribe response */
#define STATE_MQTT_DESERIALIZE_SUBACK_ERROR         (STATE_MQTT_BASE - 0x0013)
/* Failed to deserialize publish response */
#define STATE_MQTT_DESERIALIZE_PUB_ERROR            (STATE_MQTT_BASE - 0x0014)
/* Failed to deserialize unsubscribe response */
#define STATE_MQTT_DESERIALIZE_UNSUBACK_ERROR       (STATE_MQTT_BASE - 0x0015)
/* Failed to read MQTT packet from network */
#define STATE_MQTT_PACKET_READ_ERROR                (STATE_MQTT_BASE - 0x0016)
/* Failed to interpret CONNACK from MQTT server */
#define STATE_MQTT_CONNACK_UNKNOWN_ERROR            (STATE_MQTT_BASE - 0x0017)
/* Reserved buffer is too short when retrieve content from network */
#define STATE_MQTT_RX_BUFFER_TOO_SHORT              (STATE_MQTT_BASE - 0x0018)
/* Reserved buffer is too short when compose content going to network */
#define STATE_MQTT_TX_BUFFER_TOO_SHORT              (STATE_MQTT_BASE - 0x0019)
/* Reserved buffer is too short when compose topic for MQTT outgoing message */
#define STATE_MQTT_TOPIC_BUF_TOO_SHORT              (STATE_MQTT_BASE - 0x001A)
/* Retried time exceeds maximum when perform IOT_MQTT_Construct() */
#define STATE_MQTT_CONN_RETRY_EXCEED_MAX            (STATE_MQTT_BASE - 0x001B)
/* Re-publish QoS1 message exceeds maximum */
#define STATE_MQTT_QOS1_REPUB_EXCEED_MAX            (STATE_MQTT_BASE - 0x001C)
/* Invalid QoS input when publish MQTT message */
#define STATE_MQTT_PUB_QOS_INVALID                  (STATE_MQTT_BASE - 0x001D)
/* Skip current action because MQTT connection is break */
#define STATE_MQTT_IN_OFFLINE_STATUS                (STATE_MQTT_BASE - 0x001E)
/* Receive MQTT packet from network can not be interpret */
#define STATE_MQTT_RECV_UNKNOWN_PACKET              (STATE_MQTT_BASE - 0x001F)
/* MQTT connection instance exceed allowed maximum */
#define STATE_MQTT_CLI_EXCEED_MAX                   (STATE_MQTT_BASE - 0x0020)
/* MQTT subscribe instance exceed allowed maximum */
#define STATE_MQTT_SUB_EXCEED_MAX                   (STATE_MQTT_BASE - 0x0021)
/* Invalid MQTT topic can not be interpret */
#define STATE_MQTT_UNEXP_TOPIC_FORMAT               (STATE_MQTT_BASE - 0x0022)
/* Wait too long time after syncronized subscribe request sent */
#define STATE_MQTT_SYNC_SUB_TIMEOUT                 (STATE_MQTT_BASE - 0x0023)
/* Last connect request has not been responsed in async protocol stack */
#define STATE_MQTT_ASYNC_STACK_CONN_IN_PROG         (STATE_MQTT_BASE - 0x0024)
/* Currrent SDK configuration does not support working with async protocol stack */
#define STATE_MQTT_ASYNC_STACK_NOT_SUPPORT          (STATE_MQTT_BASE - 0x0025)
/* Got unknown event when work with async protocol stack */
#define STATE_MQTT_ASYNC_STACK_UNKNOWN_EVENT        (STATE_MQTT_BASE - 0x0026)
/* Report connect relative parameters such as username and password */
#define STATE_MQTT_CONN_USER_INFO                   (STATE_MQTT_BASE - 0x0027)
/* Report subscribe relative parameters such as topic string */
#define STATE_MQTT_SUB_INFO                         (STATE_MQTT_BASE - 0x0028)
/* Report publish relative parameters such as topic string */
#define STATE_MQTT_PUB_INFO                         (STATE_MQTT_BASE - 0x0029)
/* Do not get response from server, says dynamic activate request failed */
#define STATE_MQTT_DYNAMIC_ACTIVATE_FAIL_RESP       (STATE_MQTT_BASE - 0x002A)

/* MQTT: 0x0300 ~ 0x03FF */

/* COAP: 0x0500 ~ 0x05FF */
#define STATE_COAP_BASE                             (-0x0500)

/* COAP: 0x0500 ~ 0x05FF */

/* HTTP: 0x0600 ~ 0x06FF */
#define STATE_HTTP_BASE                             (-0x0600)

/* HTTP: 0x0600 ~ 0x06FF */

/* OTA: 0x0700 ~ 0x07FF */
#define STATE_OTA_BASE                              (-0x0700)

/* OTA: 0x0700 ~ 0x07FF */

/* Unused: 0x0800 ~ 0x08FF */

/* Device Model: 0x0900 ~ 0x09FF */
#define STATE_DEV_MODEL_BASE                        (-0x0900)

/* Master device already open so skip current open action */
#define STATE_DEV_MODEL_MASTER_ALREADY_OPEN         (STATE_DEV_MODEL_BASE - 0x0001)
/* Master device already connect so skip current connect action */
#define STATE_DEV_MODEL_MASTER_ALREADY_CONNECT      (STATE_DEV_MODEL_BASE - 0x0002)
/* Master device not open yet so skip current action */
#define STATE_DEV_MODEL_MASTER_NOT_OPEN_YET         (STATE_DEV_MODEL_BASE - 0x0003)
/* Master device not connect yet so skip current action */
#define STATE_DEV_MODEL_MASTER_NOT_CONNECT_YET      (STATE_DEV_MODEL_BASE - 0x0004)
/* Requested device already created */
#define STATE_DEV_MODEL_DEVICE_ALREADY_EXIST        (STATE_DEV_MODEL_BASE - 0x0005)
/* Requested device not found in list */
#define STATE_DEV_MODEL_DEVICE_NOT_FOUND            (STATE_DEV_MODEL_BASE - 0x0006)
/* Requested device can not be deleted */
#define STATE_DEV_MODEL_SUBD_NOT_DELETEABLE         (STATE_DEV_MODEL_BASE - 0x0007)
/* Requested device not in login status so can not logout */
#define STATE_DEV_MODEL_SUBD_NOT_LOGIN              (STATE_DEV_MODEL_BASE - 0x0008)
/* Internal error happens in device model function */
#define STATE_DEV_MODEL_INTERNAL_ERROR              (STATE_DEV_MODEL_BASE - 0x0009)
/* Internal event about MQTT connect happens in device model function */
#define STATE_DEV_MODEL_INTERNAL_MQTT_DUP_INIT      (STATE_DEV_MODEL_BASE - 0x000A)
/* Internal error about MQTT unconnect happens in device model function */
#define STATE_DEV_MODEL_INTERNAL_MQTT_NOT_INIT_YET  (STATE_DEV_MODEL_BASE - 0x000B)
/* Failed to open handler for cloud abstract layer */
#define STATE_DEV_MODEL_CM_OPEN_FAILED              (STATE_DEV_MODEL_BASE - 0x000C)
/* Failed to find file descriptor in cloud abstract layer */
#define STATE_DEV_MODEL_CM_FD_NOT_FOUND             (STATE_DEV_MODEL_BASE - 0x000D)
/* Internal error about file descriptor happens in device model function */
#define STATE_DEV_MODEL_CM_FD_ERROR                 (STATE_DEV_MODEL_BASE - 0x000E)
/* Failed to connect MQTT in device model function */
#define STATE_DEV_MODEL_MQTT_CONNECT_FAILED         (STATE_DEV_MODEL_BASE - 0x000F)
/* Got unexpected MQTT message from server in device model */
#define STATE_DEV_MODEL_RECV_UNEXP_MQTT_PUB         (STATE_DEV_MODEL_BASE - 0x0010)
/* Got MQTT message from server but its JSON format is wrong */
#define STATE_DEV_MODEL_WRONG_JSON_FORMAT           (STATE_DEV_MODEL_BASE - 0x0011)
/* Failed to lookup value for specified key when parse JSON */
#define STATE_DEV_MODEL_GET_JSON_ITEM_FAILED        (STATE_DEV_MODEL_BASE - 0x0012)
/* Service respond does not have correct request context in device model */
#define STATE_DEV_MODEL_SERVICE_CTX_NOT_EXIST       (STATE_DEV_MODEL_BASE - 0x0013)
/* OTA service is not enabled in device model */
#define STATE_DEV_MODEL_OTA_NOT_ENABLED             (STATE_DEV_MODEL_BASE - 0x0014)
/* OTA service is not initialized correctly in device model */
#define STATE_DEV_MODEL_OTA_NOT_INITED              (STATE_DEV_MODEL_BASE - 0x0015)
/* OTA service is initialized but failed in device model */
#define STATE_DEV_MODEL_OTA_INIT_FAILED             (STATE_DEV_MODEL_BASE - 0x0016)
/* OTA for some sub-device is not finished yet so skip device switching */
#define STATE_DEV_MODEL_OTA_STILL_IN_PROGRESS       (STATE_DEV_MODEL_BASE - 0x0017)
/* OTA firmware downloaded failed to pass integrity check */
#define STATE_DEV_MODEL_OTA_IMAGE_CHECK_FAILED      (STATE_DEV_MODEL_BASE - 0x0018)
/* OTA type is neither cota or fota */
#define STATE_DEV_MODEL_OTA_TYPE_ERROR              (STATE_DEV_MODEL_BASE - 0x0019)
/* OTA fetching from cloud failed to get expected content */
#define STATE_DEV_MODEL_OTA_FETCH_FAILED            (STATE_DEV_MODEL_BASE - 0x001A)
/* Gateway/Sub-device management function is not configured on */
#define STATE_DEV_MODEL_GATEWAY_NOT_ENABLED         (STATE_DEV_MODEL_BASE - 0x001C)
/* Configured rawdata+solo mode, will not proceed JSON messages */
#define STATE_DEV_MODEL_IN_RAWDATA_SOLO             (STATE_DEV_MODEL_BASE - 0x001E)
/* Duplicated upstream device model messages */
#define STATE_DEV_MODEL_DUP_UPSTREAM_MSG            (STATE_DEV_MODEL_BASE - 0x001F)
/* Corresponding upstream record not found for downstream messages */
#define STATE_DEV_MODEL_UPSTREAM_REC_NOT_FOUND      (STATE_DEV_MODEL_BASE - 0x0020)
/* Got negative respond from cloud in device model */
#define STATE_DEV_MODEL_REFUSED_BY_CLOUD            (STATE_DEV_MODEL_BASE - 0x0021)
/* Encount unexpected option when invoke dm_opt_get() */
#define STATE_DEV_MODEL_INVALID_DM_OPTION           (STATE_DEV_MODEL_BASE - 0x0022)
/* Encount parsing failure when process URL in respond from cloud */
#define STATE_DEV_MODEL_URL_SPLIT_FAILED            (STATE_DEV_MODEL_BASE - 0x0023)
/* Failed to parse EnOS request or respond messages */
#define STATE_DEV_MODEL_ENOS_MSG_PARSE_FAILED      (STATE_DEV_MODEL_BASE - 0x0024)
/* Failed to upload device log to cloud */
#define STATE_DEV_MODEL_LOG_REPORT_ERROR            (STATE_DEV_MODEL_BASE - 0x0025)
/* Bypass current topic subscription since auto subscribed */
#define STATE_DEV_MODEL_IN_AUTOSUB_MODE             (STATE_DEV_MODEL_BASE - 0x0026)
/* Got message from cloud in device model */
#define STATE_DEV_MODEL_RX_CLOUD_MESSAGE            (STATE_DEV_MODEL_BASE - 0x0027)
/* Sending message to cloud in device model */
#define STATE_DEV_MODEL_TX_CLOUD_MESSAGE            (STATE_DEV_MODEL_BASE - 0x0028)
/* Got message from local in device model */
#define STATE_DEV_MODLE_RX_LOCAL_MESSAGE            (STATE_DEV_MODEL_BASE - 0x0029)
/* Sending message to local in device model */
#define STATE_DEV_MODLE_TX_LOCAL_MESSAGE            (STATE_DEV_MODEL_BASE - 0x002A)
/* Processing message queue of device model and external modules */
#define STATE_DEV_MODEL_MSGQ_OPERATION              (STATE_DEV_MODEL_BASE - 0x002C)
/* Message queue in device model is full */
#define STATE_DEV_MODEL_MSGQ_FULL                   (STATE_DEV_MODEL_BASE - 0x002D)
/* Message queue in device model is empty */
#define STATE_DEV_MODEL_MSGQ_EMPTY                  (STATE_DEV_MODEL_BASE - 0x002E)
/* Cache list in device model get inserted new element */
#define STATE_DEV_MODEL_CTX_LIST_INSERT             (STATE_DEV_MODEL_BASE - 0x002F)
/* Cache list in device model get removed element */
#define STATE_DEV_MODEL_CTX_LIST_REMOVE             (STATE_DEV_MODEL_BASE - 0x0030)
/* Element in cache list of device model removed since life-time over */
#define STATE_DEV_MODEL_CTX_LIST_FADEOUT            (STATE_DEV_MODEL_BASE - 0x0031)
/* Cache list in device model get filled full */
#define STATE_DEV_MODEL_CTX_LIST_FULL               (STATE_DEV_MODEL_BASE - 0x0032)
/* Cache list in device model get consumed empty */
#define STATE_DEV_MODEL_CTX_LIST_EMPTY              (STATE_DEV_MODEL_BASE - 0x0033)
/* Log post to cloud get stopped */
#define STATE_DEV_MODLE_LOG_REPORT_STOP             (STATE_DEV_MODEL_BASE - 0x0034)
/* Log post to cloud get switched */
#define STATE_DEV_MODLE_LOG_REPORT_SWITCH           (STATE_DEV_MODEL_BASE - 0x0035)
/* Log post to cloud get transmitted */
#define STATE_DEV_MODLE_LOG_REPORT_SEND             (STATE_DEV_MODEL_BASE - 0x0036)
/* Operating to list of sync request in device model */
#define STATE_DEV_MODEL_SYNC_REQ_LIST               (STATE_DEV_MODEL_BASE - 0x0037)
/* Event of EnOS protocol processing is happenning */
#define STATE_DEV_MODEL_ENOS_PROT_EVENT             (STATE_DEV_MODEL_BASE - 0x0038)
/* Got invalid EnOS protocol mqtt packet */
#define STATE_DEV_MODEL_INVALID_ENOS_PAYLOAD        (STATE_DEV_MODEL_BASE - 0x003A)
/* Got invalid requested EnOS protocol mqtt topic */
#define STATE_DEV_MODEL_INVALID_ENOS_TOPIC          (STATE_DEV_MODEL_BASE - 0x003B)
/* IOT_EnOS_Yield already stopped */
#define STATE_DEV_MODEL_YIELD_STOPPED               (STATE_DEV_MODEL_BASE - 0x003C)
/* IOT_EnOS_Yield still running */
#define STATE_DEV_MODEL_YIELD_RUNNING               (STATE_DEV_MODEL_BASE - 0x003D)

/* Device Model: 0x0900 ~ 0x09FF */

#ifdef __cplusplus
}
#endif
#endif  /* __INFRA_STATE_H__ */

