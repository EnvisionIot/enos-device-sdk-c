/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */

#ifndef _IOT_EXPORT_ENOS_H_
#define _IOT_EXPORT_ENOS_H_

#if defined(__cplusplus)
extern "C"
{
#endif

#include "infra_types.h"
#include "infra_defs.h"

   typedef enum
   {
      IOTX_ENOS_DEV_TYPE_MASTER,
      IOTX_ENOS_DEV_TYPE_SLAVE,
      IOTX_ENOS_DEV_TYPE_MAX
   } iotx_enos_dev_type_t;

   typedef struct
   {
      char product_key[IOTX_PRODUCT_KEY_LEN + 1];
      char product_secret[IOTX_PRODUCT_SECRET_LEN + 1];
      char device_key[IOTX_DEVICE_KEY_LEN + 1];
      char device_secret[IOTX_DEVICE_SECRET_LEN + 1];
   } iotx_enos_dev_meta_info_t;

   typedef enum
   {
      IOTX_COMMAND_MEASUREPOINT_SET,
      IOTX_COMMAND_SERVICE_INVOKE
   } iotx_command_type_t;

   typedef enum
   {

      /* Enos Tags */
      ITM_MSG_QUERY_TAG,
      ITM_MSG_UPDATE_TAG,
      ITM_MSG_DELETE_TAG,

      /* EnOS ThingModel Attributes */
      ITM_MSG_QUERY_ATTRIBUTE,
      ITM_MSG_UPDATE_ATTRIBUTE,
      ITM_MSG_DELETE_ATTRIBUTE,

      /* EnOS ThingModel Measurepoints */
      ITM_MSG_POST_MEASUREPOINT,
      ITM_MSG_POST_MEASUREPOINT_BATCH,

      /* EnOS ThingModel EVENT */
      ITM_MSG_POST_EVENT,

#ifdef DEVICE_MEASUREPOINT_RESUME
      /* measurepoint resume */
      ITM_MSG_RESUME_MEASUREPOINT,
      ITM_MSG_RESUME_MEASUREPOINT_BATCH,
#endif

      /* post raw data to cloud */
      ITM_MSG_POST_RAW_DATA,

#ifdef DEVICE_MODEL_GATEWAY
      /* only for master device, send register device request to cloud */
      ITM_MSG_REGISTER,

      /* only for slave device, send login request to cloud */
      ITM_MSG_LOGIN,

      /* only for slave device, send logout request to cloud */
      ITM_MSG_LOGOUT,

      /* only for master device, send login batch request to cloud */
      ITM_MSG_LOGIN_BATCH,

      /* only for slave device, send add topo request to cloud */
      ITM_MSG_ADD_TOPO,

      /* only for slave device, send delete topo request to cloud */
      ITM_MSG_DELETE_TOPO,

      /* only for master device, get topo list */
      ITM_MSG_GET_TOPO,

      /* report subdev's firmware version */
      ITM_MSG_REPORT_SUBDEV_FIRMWARE_VERSION,
#endif

      /* post historical measurepoint or event to cloud */
      ITM_MSG_MEASUREPOINT_RESUME_DATA,

      IOTX_ENOS_MSG_MAX
   } iotx_enos_msg_type_t;

   typedef struct
   {
      int msg_type;
      char *method;
   } iotx_msg_type_method_map_t;

   /**
    * @brief create a new device
    *
    * @param dev_type. type of device which will be created. see iotx_enos_dev_type_t
    * @param meta_info. The product key, product secret, device key and device secret of new device.
    *
    * @return success: device id (>=0), fail: -1.
    *
    */
   int IOT_EnOS_Open(iotx_enos_dev_type_t dev_type, iotx_enos_dev_meta_info_t *meta_info);

   /**
    * @brief start device network connection.
    *        for master device, start to connect enos server.
    *        for slave device, send message to cloud for register new device and add topo with master device
    *
    * @param devid. device identifier.
    *
    * @return success: device id (>=0), fail: -1.
    *
    */
   int IOT_EnOS_Connect(int devid);

   /**
    * @brief try to receive message from cloud and dispatch these message to user event callback
    *
    * @param timeout_ms. timeout for waiting new message arrived
    *
    * @return state code.
    *
    */
   int IOT_EnOS_Yield(int timeout_ms);

   /**
    * @brief close device network connection and release resources.
    *        for master device, disconnect with EnOS server and release all local resources.
    *        for slave device, send message to cloud for delete topo with master device and unregister itself, then release device's resources.
    *
    * @param devid. device identifier.
    *
    * @return success: 0, fail: -1.
    *
    */
   int IOT_EnOS_Close(int devid);

   /**
    * @brief Report message to cloud
    *
    * @param devid. device identifier.
    * @param msg_type. message type. see iotx_enos_msg_type_t, as follows:
    *        ITM_MSG_POST_MEASUREPOINT
    *        ITM_MSG_POST_RAW_DATA
    *        ITM_MSG_LOGIN
    *        ITM_MSG_LOGOUT
    *
    * @param payload. message payload.
    * @param payload_len. message payload length.
    *
    * @return success: 0 or message id (>=1), fail: -1.
    *
    */
   int IOT_EnOS_Report(int devid, iotx_enos_msg_type_t msg_type, unsigned char *payload,
                       int payload_len);

   /**
    * @brief Reply message to cloud
    * @param devid. device identifier
    * @param msg_type. message type. see iotx_enos_msg_type_t, as follows:
    *        ITM_MSG_SET_MEASUREPOINT_REPLY,
    *        ITM_MSG_INVOKE_SERVICE_REPLY,
    * @return success: 0 or message id (>=1), fail: -1.             
    */
   /* int IOT_EnOS_Reply(int devid, iotx_enos_msg_type_t msg_type, unsigned char *payload,
                                   int payload_len); */

   /**
    * @brief post message to cloud
    *
    * @param devid. device identifier.
    * @param msg_type. message type. see iotx_enos_msg_type_t, as follows:
    *        ITM_MSG_QUERY_TIMESTAMP
    *        ITM_MSG_GET_TOPO
    *
    * @param payload. message payload.
    * @param payload_len. message payload length.
    *
    * @return success: 0 or message id (>=1), fail: -1.
    *
    */
   /* int IOT_EnOS_Query(int devid, iotx_enos_msg_type_t msg_type, unsigned char *payload,
                                  int payload_len); */

   /**
    * @brief post event to cloud
    *
    * @param devid. device identifier.
    * @param eventid. tsl event id.
    * @param eventid_len. length of tsl event id.
    * @param payload. event payload.
    * @param payload_len. event payload length.
    *
    * @return success: message id (>=1), fail: -1.
    *
    */
   int IOT_EnOS_TriggerEvent(int devid, char *eventid, int eventid_len, char *payload, int payload_len);

   /**
    * @brief post service response to cloud
    *
    * @param devid. device identifier.
    * @param command_type. command type: measurepoint set or service invoke
    * @param serviceid. tsl service id.
    * @param serviceid_len. length of tsl service id.
    * @param payload. service response payload.
    * @param payload_len. service response payload length.
    *
    * @return success: 0, fail: -1.
    *
    */
   int IOT_EnOS_CommandReply(int devid,
                             iotx_command_type_t command_type, char *serviceid, int serviceid_len,
                             char *payload, int payload_len, void *p_service_ctx);

#if defined(__cplusplus)
}
#endif
#endif
