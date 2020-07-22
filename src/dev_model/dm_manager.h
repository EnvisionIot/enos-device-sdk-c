/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */



#ifndef _DM_MANAGER_H_
#define _DM_MANAGER_H_

#include "iotx_dm_internal.h"

typedef struct {
    int devid;
    int dev_type;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1];
    char product_secret[IOTX_PRODUCT_SECRET_LEN + 1];
    char device_key[IOTX_DEVICE_KEY_LEN + 1];
    char device_secret[IOTX_DEVICE_SECRET_LEN + 1];
    iotx_dm_dev_avail_t status;
    iotx_dm_dev_status_t dev_status;
    struct list_head linked_list;
} dm_mgr_dev_node_t;

typedef struct {
    void *mutex;
    int global_devid;
    struct list_head dev_list;
} dm_mgr_ctx;

int dm_mgr_init(void);
int dm_mgr_deinit(void);
int dm_mgr_device_query(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1], _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1], _OU_ int *devid);
int dm_mgr_device_create(_IN_ int dev_type, _IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                         _IN_ char product_secret[IOTX_PRODUCT_SECRET_LEN + 1],
                         _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1], _IN_ char device_secret[IOTX_DEVICE_SECRET_LEN + 1], _OU_ int *devid);
int dm_mgr_device_destroy(_IN_ int devid);
int dm_mgr_device_number(void);
int dm_mgr_get_devid_by_index(_IN_ int index, _OU_ int *devid);
int dm_mgr_get_next_devid(_IN_ int devid, _OU_ int *devid_next);
int dm_mgr_search_device_by_devid(_IN_ int devid, _OU_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                                  _OU_ char device_key[IOTX_DEVICE_KEY_LEN + 1], _OU_ char device_secret[IOTX_DEVICE_SECRET_LEN + 1]);
int dm_mgr_search_device_by_pkdk(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                                 _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                                 _OU_ int *devid);
int dm_mgr_search_device_node_by_devid(_IN_ int devid, _OU_ void **node);

int dm_mgr_get_dev_type(_IN_ int devid, _OU_ int *dev_type);
int dm_mgr_set_dev_enable(_IN_ int devid);
int dm_mgr_set_dev_disable(_IN_ int devid);
int dm_mgr_get_dev_avail(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                         _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                         _OU_ iotx_dm_dev_avail_t *status);
int dm_mgr_set_dev_status(_IN_ int devid, _IN_ iotx_dm_dev_status_t status);
int dm_mgr_get_dev_status(_IN_ int devid, _OU_ iotx_dm_dev_status_t *status);
int dm_mgr_set_device_secret(_IN_ int devid, _IN_ char device_secret[IOTX_DEVICE_SECRET_LEN + 1]);
int dm_mgr_dev_initialized(int devid);


#ifdef DEVICE_MODEL_GATEWAY
    int dm_mgr_upstream_thing_device_register(_IN_ int devid);
    int dm_mgr_upstream_thing_topo_add(_IN_ int devid);
    int dm_mgr_upstream_thing_topo_delete(_IN_ int devid);
    int dm_mgr_upstream_thing_topo_get(void);
    int dm_mgr_upstream_combine_login(_IN_ int devid);
    int dm_mgr_upstream_combine_login_batch(_IN_ int devid, _IN_ int* sub_devids, _IN_ int sub_devids_len);
    int dm_mgr_upstream_combine_logout(_IN_ int devid);
#endif
int dm_mgr_upstream_thing_model_up_raw(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len);

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)

int dm_mgr_upstream_common(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len, _IN_ char* method);

#ifdef DEVICE_MEASUREPOINT_RESUME
int dm_mgr_upstream_thing_measurepoint_resume(int devid, char *payload, int payload_len);
#endif /* #ifdef DEVICE_MEASUREPOINT_RESUME */

int dm_mgr_upstream_thing_event_post(_IN_ int devid, _IN_ char *identifier, _IN_ int identifier_len, _IN_ char *method,
        _IN_ char *payload, _IN_ int payload_len);

int dm_mgr_upstream_thing_measurepoint_set_response(_IN_ int devid, _IN_ char *msgid, _IN_ int msgid_len,
        _IN_ iotx_dm_error_code_t code, _IN_ char *payload, _IN_ int payload_len, void *ctx);

int dm_mgr_upstream_thing_service_invoke_response(_IN_ int devid, _IN_ char *msgid, _IN_ int msgid_len,
        _IN_ iotx_dm_error_code_t code, _IN_ char *identifier, _IN_ int identifier_len,
        _IN_ char *payload, _IN_ int payload_len, void *ctx);

#ifdef DEVICE_MODEL_SUBDEV_OTA
    int dm_mgr_upstream_thing_firmware_version_update(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len);
#endif

#endif

#endif
