/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */



#include "iotx_dm_internal.h"

static dm_mgr_ctx g_dm_mgr = {0};

static dm_mgr_ctx *_dm_mgr_get_ctx(void)
{
    return &g_dm_mgr;
}

static void _dm_mgr_mutex_lock(void)
{
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();
    if (ctx->mutex) {
        HAL_MutexLock(ctx->mutex);
    }
}

static void _dm_mgr_mutex_unlock(void)
{
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();
    if (ctx->mutex) {
        HAL_MutexUnlock(ctx->mutex);
    }
}

static int _dm_mgr_next_devid(void)
{
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();

    return ctx->global_devid++;
}

static int _dm_mgr_search_dev_by_devid(_IN_ int devid, _OU_ dm_mgr_dev_node_t **node)
{
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();
    dm_mgr_dev_node_t *search_node = NULL;

    list_for_each_entry(search_node, &ctx->dev_list, linked_list, dm_mgr_dev_node_t) {
        if (search_node->devid == devid) {
            if (node) {
                *node = search_node;
            }
            return SUCCESS_RETURN;
        }
    }

    return STATE_DEV_MODEL_DEVICE_NOT_FOUND;
}

static int _dm_mgr_search_dev_by_pkdn(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                                      _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1], _OU_ dm_mgr_dev_node_t **node)
{
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();
    dm_mgr_dev_node_t *search_node = NULL;

    list_for_each_entry(search_node, &ctx->dev_list, linked_list, dm_mgr_dev_node_t) {
        if ((strlen(search_node->product_key) == strlen(product_key)) &&
            (memcmp(search_node->product_key, product_key, strlen(product_key)) == 0) &&
            (strlen(search_node->device_key) == strlen(device_key)) &&
            (memcmp(search_node->device_key, device_key, strlen(device_key)) == 0)) {
            if (node) {
                *node = search_node;
            }
            return SUCCESS_RETURN;
        }
    }

    return STATE_DEV_MODEL_DEVICE_NOT_FOUND;
}

static int _dm_mgr_insert_dev(_IN_ int devid, _IN_ int dev_type, char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                              char device_key[IOTX_DEVICE_KEY_LEN + 1])
{
    int res = 0;
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();
    dm_mgr_dev_node_t *node = NULL;

    if (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) {
        return STATE_USER_INPUT_PK;
    }

    if (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1) {
        return STATE_USER_INPUT_DN;
    }

    res = _dm_mgr_search_dev_by_devid(devid, NULL);
    if (res == SUCCESS_RETURN) {
        return STATE_DEV_MODEL_DEVICE_ALREADY_EXIST;
    }

    node = DM_malloc(sizeof(dm_mgr_dev_node_t));
    if (node == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(node, 0, sizeof(dm_mgr_dev_node_t));

    node->devid = devid;
    node->dev_type = dev_type;
    memcpy(node->product_key, product_key, strlen(product_key));
    memcpy(node->device_key, device_key, strlen(device_key));
    INIT_LIST_HEAD(&node->linked_list);

    list_add_tail(&node->linked_list, &ctx->dev_list);

    return SUCCESS_RETURN;
}

static void _dm_mgr_destroy_devlist(void)
{
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();
    dm_mgr_dev_node_t *del_node = NULL;
    dm_mgr_dev_node_t *next_node = NULL;

    list_for_each_entry_safe(del_node, next_node, &ctx->dev_list, linked_list, dm_mgr_dev_node_t) {
        list_del(&del_node->linked_list);
        DM_free(del_node);
    }
}

int dm_mgr_init(void)
{
    int res = 0;
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};

    memset(ctx, 0, sizeof(dm_mgr_ctx));

    /* Create Mutex */
    ctx->mutex = HAL_MutexCreate();
    if (ctx->mutex == NULL) {
        res = STATE_SYS_DEPEND_MUTEX_CREATE;
        goto ERROR;
    }

    /* Init Device Id*/
    ctx->global_devid = IOTX_DM_LOCAL_NODE_DEVID + 1;

    /* Init Device List */
    INIT_LIST_HEAD(&ctx->dev_list);

    /* Local Node */
    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_KEY, device_key);
    res = _dm_mgr_insert_dev(IOTX_DM_LOCAL_NODE_DEVID, IOTX_DM_DEVICE_TYPE, product_key, device_key);
    if (res != SUCCESS_RETURN) {
        goto ERROR;
    }

    return SUCCESS_RETURN;

ERROR:
    if (ctx->mutex) {
        HAL_MutexDestroy(ctx->mutex);
    }
    memset(ctx, 0, sizeof(dm_mgr_ctx));
    return res;
}

int dm_mgr_deinit(void)
{
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();

    _dm_mgr_mutex_lock();
    _dm_mgr_destroy_devlist();
    _dm_mgr_mutex_unlock();

    if (ctx->mutex) {
        HAL_MutexDestroy(ctx->mutex);
    }

    return SUCCESS_RETURN;
}

int dm_mgr_device_query(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                        _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1], _OU_ int *devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    /* duplicated parameters check is removed */

    res = _dm_mgr_search_dev_by_pkdn(product_key, device_key, &node);
    if (res == SUCCESS_RETURN) {
        if (devid) {
            *devid = node->devid;
        }
        return SUCCESS_RETURN;
    }

    return STATE_DEV_MODEL_DEVICE_NOT_FOUND;
}

int dm_mgr_device_create(_IN_ int dev_type, _IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                         _IN_ char product_secret[IOTX_PRODUCT_SECRET_LEN + 1],
                         _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1], _IN_ char device_secret[IOTX_DEVICE_SECRET_LEN + 1], _OU_ int *devid)
{
    int res = 0;
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();
    dm_mgr_dev_node_t *node = NULL;

    if (product_key == NULL || product_secret == NULL || device_key == NULL ||
        strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1 ||
        strlen(product_secret) >= IOTX_PRODUCT_SECRET_LEN + 1 ||
        strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1) {
        return STATE_USER_INPUT_INVALID;
    }

    if (device_secret != NULL && strlen(device_secret) >= IOTX_DEVICE_SECRET_LEN + 1) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_pkdn(product_key, device_key, &node);
    if (res == SUCCESS_RETURN) {
        if (devid) {
            *devid = node->devid;
        }
        return STATE_DEV_MODEL_DEVICE_ALREADY_EXIST;
    }

    node = DM_malloc(sizeof(dm_mgr_dev_node_t));
    if (node == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(node, 0, sizeof(dm_mgr_dev_node_t));

    node->devid = _dm_mgr_next_devid();
    node->dev_type = dev_type;
    memcpy(node->product_key, product_key, strlen(product_key));
    memcpy(node->product_secret, product_secret, strlen(product_secret));
    memcpy(node->device_key, device_key, strlen(device_key));
    if (device_secret != NULL) {
        memcpy(node->device_secret, device_secret, strlen(device_secret));
    }
    node->dev_status = IOTX_DM_DEV_STATUS_AUTHORIZED;
    INIT_LIST_HEAD(&node->linked_list);

    list_add_tail(&node->linked_list, &ctx->dev_list);

    if (devid) {
        *devid = node->devid;
    }

    return SUCCESS_RETURN;
}

int dm_mgr_device_destroy(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    if (devid < 0) {
        return STATE_USER_INPUT_DEVID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    if (node->devid == IOTX_DM_LOCAL_NODE_DEVID) {
        return STATE_DEV_MODEL_SUBD_NOT_DELETEABLE;
    }

    list_del(&node->linked_list);

#ifdef DEVICE_MODEL_GATEWAY
    dm_client_subdev_unsubscribe(node->product_key, node->device_key);
#endif

    DM_free(node);

    return SUCCESS_RETURN;
}

int dm_mgr_device_number(void)
{
    int index = 0;
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();
    dm_mgr_dev_node_t *search_node = NULL;

    list_for_each_entry(search_node, &ctx->dev_list, linked_list, dm_mgr_dev_node_t) {
        index++;
    }

    return index;
}

int dm_mgr_get_devid_by_index(_IN_ int index, _OU_ int *devid)
{
    int search_index = 0;
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();
    dm_mgr_dev_node_t *search_node = NULL;

    if (index < 0 || devid == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    list_for_each_entry(search_node, &ctx->dev_list, linked_list, dm_mgr_dev_node_t) {
        if (search_index == index) {
            *devid = search_node->devid;
            return SUCCESS_RETURN;
        }
        search_index++;
    }

    return STATE_DEV_MODEL_DEVICE_NOT_FOUND;
}

int dm_mgr_get_next_devid(_IN_ int devid, _OU_ int *devid_next)
{
    dm_mgr_ctx *ctx = _dm_mgr_get_ctx();
    dm_mgr_dev_node_t *search_node = NULL;
    dm_mgr_dev_node_t *next_node = NULL;

    if (devid < 0 || devid_next == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    list_for_each_entry(next_node, &ctx->dev_list, linked_list, dm_mgr_dev_node_t) {
        if (search_node && search_node->devid == devid) {
            *devid_next = next_node->devid;
            return SUCCESS_RETURN;
        }

        if (next_node->devid == devid) {
            search_node = next_node;
        }
    }

    return STATE_DEV_MODEL_DEVICE_NOT_FOUND;
}

int dm_mgr_search_device_by_devid(_IN_ int devid, _OU_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                                  _OU_ char device_key[IOTX_DEVICE_KEY_LEN + 1], _OU_ char device_secret[IOTX_DEVICE_SECRET_LEN + 1])
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    if (product_key == NULL || device_key == NULL || device_secret == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    memcpy(product_key, node->product_key, strlen(node->product_key));
    memcpy(device_key, node->device_key, strlen(node->device_key));
    memcpy(device_secret, node->device_secret, strlen(node->device_secret));

    return SUCCESS_RETURN;
}

int dm_mgr_search_device_by_pkdn(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                                 _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                                 _OU_ int *devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    if (product_key == NULL || device_key == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_pkdn(product_key, device_key, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    if (devid) {
        *devid = node->devid;
    }

    return SUCCESS_RETURN;
}

int dm_mgr_search_device_node_by_devid(_IN_ int devid, _OU_ void **node)
{
    int res = 0;
    dm_mgr_dev_node_t *search_node = NULL;

    res = _dm_mgr_search_dev_by_devid(devid, &search_node);
    if (res != SUCCESS_RETURN) {
        return STATE_DEV_MODEL_DEVICE_NOT_FOUND;
    }

    if (node) {
        *node = (void *)search_node;
    }

    return SUCCESS_RETURN;
}

int dm_mgr_get_dev_type(_IN_ int devid, _OU_ int *dev_type)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    if (devid < 0 || dev_type == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    *dev_type = node->dev_type;

    return SUCCESS_RETURN;
}

int dm_mgr_set_dev_enable(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    if (devid < 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    node->status = IOTX_DM_DEV_AVAIL_ENABLE;

    return SUCCESS_RETURN;
}

int dm_mgr_set_dev_disable(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    if (devid < 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    node->status = IOTX_DM_DEV_AVAIL_DISABLE;

    return SUCCESS_RETURN;
}

int dm_mgr_get_dev_avail(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                         _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                         _OU_ iotx_dm_dev_avail_t *status)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    if (product_key == NULL || device_key == NULL || status == NULL ||
        (strlen(product_key) >= IOTX_PRODUCT_KEY_LEN + 1) ||
        (strlen(device_key) >= IOTX_DEVICE_KEY_LEN + 1)) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_pkdn(product_key, device_key, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    *status = node->status;

    return SUCCESS_RETURN;
}

int dm_mgr_set_dev_status(_IN_ int devid, _IN_ iotx_dm_dev_status_t status)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    if (devid < 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    node->dev_status = status;

    return SUCCESS_RETURN;
}

int dm_mgr_get_dev_status(_IN_ int devid, _OU_ iotx_dm_dev_status_t *status)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    if (devid < 0 || status == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    *status = node->dev_status;

    return SUCCESS_RETURN;
}

int dm_mgr_set_device_secret(_IN_ int devid, _IN_ char device_secret[IOTX_DEVICE_SECRET_LEN + 1])
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    if (devid < 0 || device_secret == NULL ||
        strlen(device_secret) >= IOTX_DEVICE_SECRET_LEN + 1) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    memset(node->device_secret, 0, IOTX_DEVICE_SECRET_LEN + 1);
    memcpy(node->device_secret, device_secret, strlen(device_secret));

    return SUCCESS_RETURN;
}

int dm_mgr_dev_initialized(int devid)
{
    int res = 0, message_len = 0;
    char *message = NULL;
    const char *fmt = "{\"devid\":%d}";

    message_len = strlen(fmt) + DM_UTILS_UINT32_STRLEN + 1;
    message = DM_malloc(message_len);
    if (message == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(message, 0, message_len);
    HAL_Snprintf(message, message_len, fmt, devid);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_INITIALIZED, message);
    if (res != SUCCESS_RETURN) {
        DM_free(message);
        return res;
    }

    return SUCCESS_RETURN;
}

#ifdef DEVICE_MODEL_GATEWAY
int dm_mgr_upstream_thing_sub_register(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;
    dm_msg_request_t request;

    if (devid < 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    memset(&request, 0, sizeof(dm_msg_request_t));
    request.reply_prefix = DM_URI_SYS_PREFIX;
    request.reply_name = DM_URI_THING_SUB_REGISTER;
    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, request.product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_KEY, request.device_key);

    /* Get Params And Method */
    res = dm_msg_thing_sub_register(node->product_key, node->device_key, &request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Get Msg ID */
    request.msgid = iotx_report_id();

    /* Get Dev ID */
    request.devid = devid;

    /* Callback */
    request.callback = dm_client_thing_sub_register_reply;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_insert(request.msgid, request.devid, IOTX_DM_EVENT_SUBDEV_REGISTER_REPLY, NULL);
#endif
    /* Send Message To Cloud */
    res = dm_msg_request(DM_MSG_DEST_CLOUD, &request);
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (res != SUCCESS_RETURN) {
        dm_msg_cache_remove(request.msgid);
    } else {
        res = request.msgid;
    }
#endif
    DM_free(request.params);

    return res;
}

int dm_mgr_upstream_thing_proxy_product_register(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;
    dm_msg_request_t request;

    if (devid < 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    memset(&request, 0, sizeof(dm_msg_request_t));
    request.reply_prefix = DM_URI_SYS_PREFIX;
    request.reply_name = DM_URI_THING_PROXY_PRODUCT_REGISTER;
    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, request.product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_KEY, request.device_key);

    /* Get Params And Method */
    res = dm_msg_thing_proxy_product_register(node->product_key, node->product_secret, node->device_key, &request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Get Msg ID */
    request.msgid = iotx_report_id();

    /* Get Dev ID */
    request.devid = devid;

    /* Callback */
    request.callback = dm_client_thing_proxy_product_register_reply;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_insert(request.msgid, request.devid, IOTX_DM_EVENT_SUBDEV_REGISTER_REPLY, NULL);
#endif
    /* Send Message To Cloud */
    res = dm_msg_request(DM_MSG_DEST_CLOUD, &request);
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (res != SUCCESS_RETURN) {
        dm_msg_cache_remove(request.msgid);
    } else {
        res = request.msgid;
    }
#endif
    DM_free(request.params);

    return res;
}

int dm_mgr_upstream_thing_sub_unregister(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;
    dm_msg_request_t request;

    if (devid < 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    memset(&request, 0, sizeof(dm_msg_request_t));
    request.reply_prefix = DM_URI_SYS_PREFIX;
    request.reply_name = DM_URI_THING_SUB_UNREGISTER;
    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, request.product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_KEY, request.device_key);

    /* Get Params And Method */
    res = dm_msg_thing_sub_unregister(node->product_key, node->device_key, &request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Get Msg ID */
    request.msgid = iotx_report_id();

    /* Get Dev ID */
    request.devid = devid;

    /* Callback */
    request.callback = dm_client_thing_sub_unregister_reply;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_insert(request.msgid, request.devid, IOTX_DM_EVENT_SUBDEV_UNREGISTER_REPLY, NULL);
#endif
    /* Send Message To Cloud */
    res = dm_msg_request(DM_MSG_DEST_CLOUD, &request);
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (res != SUCCESS_RETURN) {
        dm_msg_cache_remove(request.msgid);
    } else {
        res = request.msgid;
    }
#endif
    DM_free(request.params);

    return res;
}

int dm_mgr_upstream_thing_topo_add(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;
    dm_msg_request_t request;

    if (devid < 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    memset(&request, 0, sizeof(dm_msg_request_t));
    request.reply_prefix = DM_URI_SYS_PREFIX;
    request.reply_name = DM_URI_THING_TOPO_ADD;
    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, request.product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_KEY, request.device_key);

    /* Get Params And Method */
    res = dm_msg_thing_topo_add(node->product_key, node->device_key, node->device_secret, &request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Get Msg ID */
    request.msgid = iotx_report_id();

    /* Get Dev ID */
    request.devid = devid;

    /* Callback */
    request.callback = dm_client_thing_topo_add_reply;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_insert(request.msgid, request.devid, IOTX_DM_EVENT_TOPO_ADD_REPLY, NULL);
#endif
    /* Send Message To Cloud */
    res = dm_msg_request(DM_MSG_DEST_CLOUD, &request);
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (res != SUCCESS_RETURN) {
        dm_msg_cache_remove(request.msgid);
    } else {
        res = request.msgid;
    }
#endif
    DM_free(request.params);

    return res;
}

int dm_mgr_upstream_thing_topo_delete(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;
    dm_msg_request_t request;

    if (devid < 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    memset(&request, 0, sizeof(dm_msg_request_t));
    request.reply_prefix = DM_URI_SYS_PREFIX;
    request.reply_name = DM_URI_THING_TOPO_DELETE;
    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, request.product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_KEY, request.device_key);

    /* Get Params And Method */
    res = dm_msg_thing_topo_delete(node->product_key, node->device_key, &request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Get Msg ID */
    request.msgid = iotx_report_id();

    /* Get Dev ID */
    request.devid = devid;

    /* Callback */
    request.callback = dm_client_thing_topo_delete_reply;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_insert(request.msgid, request.devid, IOTX_DM_EVENT_TOPO_DELETE_REPLY, NULL);
#endif
    /* Send Message To Cloud */
    res = dm_msg_request(DM_MSG_DEST_CLOUD, &request);
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (res != SUCCESS_RETURN) {
        dm_msg_cache_remove(request.msgid);
    } else {
        res = request.msgid;
    }
#endif
    DM_free(request.params);

    return res;
}

int dm_mgr_upstream_thing_topo_get(void)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;
    dm_msg_request_t request;

    memset(&request, 0, sizeof(dm_msg_request_t));
    request.reply_prefix = DM_URI_SYS_PREFIX;
    request.reply_name = DM_URI_THING_TOPO_GET;
    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, request.product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_KEY, request.device_key);

    res = _dm_mgr_search_dev_by_pkdn(request.product_key, request.device_key, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Get Params And Method */
    res = dm_msg_thing_topo_get(&request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Get Msg ID */
    request.msgid = iotx_report_id();

    /* Get Dev ID */
    request.devid = node->devid;

    /* Callback */
    request.callback = dm_client_thing_topo_get_reply;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_insert(request.msgid, request.devid, IOTX_DM_EVENT_TOPO_GET_REPLY, NULL);
#endif
    /* Send Message To Cloud */
    res = dm_msg_request(DM_MSG_DEST_CLOUD, &request);
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (res != SUCCESS_RETURN) {
        dm_msg_cache_remove(request.msgid);
    } else {
        res = request.msgid;
    }
#endif
    DM_free(request.params);

    return res;
}

int dm_mgr_upstream_thing_list_found(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;
    dm_msg_request_t request;

    if (devid < 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    memset(&request, 0, sizeof(dm_msg_request_t));
    request.reply_prefix = DM_URI_SYS_PREFIX;
    request.reply_name = DM_URI_THING_LIST_FOUND;
    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, request.product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_KEY, request.device_key);

    /* Get Params And Method */
    res = dm_msg_thing_list_found(node->product_key, node->device_key, &request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Get Msg ID */
    request.msgid = iotx_report_id();

    /* Get Dev ID */
    request.devid = devid;

    /* Callback */
    request.callback = dm_client_thing_list_found_reply;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_insert(request.msgid, request.devid, IOTX_DM_EVENT_TOPO_ADD_NOTIFY_REPLY, NULL);
#endif
    /* Send Message To Cloud */
    res = dm_msg_request(DM_MSG_DEST_CLOUD, &request);
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (res != SUCCESS_RETURN) {
        dm_msg_cache_remove(request.msgid);
    } else {
        res = request.msgid;
    }
#endif
    DM_free(request.params);

    return res;
}

int dm_mgr_upstream_combine_login(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;
    dm_msg_request_t request;

    if (devid < 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    memset(&request, 0, sizeof(dm_msg_request_t));
    request.reply_prefix = DM_URI_EXT_SESSION_PREFIX;
    request.reply_name = DM_URI_COMBINE_LOGIN;
    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, request.product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_KEY, request.device_key);

    /* Get Params And Method */
    res = dm_msg_combine_login(node->product_key, node->device_key, node->device_secret, &request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Get Msg ID */
    request.msgid = iotx_report_id();

    /* Get Dev ID */
    request.devid = devid;

    /* Callback */
    request.callback = dm_client_combine_login_reply;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_insert(request.msgid, request.devid, IOTX_DM_EVENT_COMBINE_LOGIN_REPLY, NULL);
#endif
    /* Send Message To Cloud */
    res = dm_msg_request(DM_MSG_DEST_CLOUD, &request);
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (res != SUCCESS_RETURN) {
        dm_msg_cache_remove(request.msgid);
    } else {
        res = request.msgid;
    }
#endif
    DM_free(request.params);

    return res;
}

int dm_mgr_upstream_combine_logout(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;
    dm_msg_request_t request;

    if (devid < 0) {
        return STATE_USER_INPUT_DEVID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    if (node->dev_status < IOTX_DM_DEV_STATUS_LOGINED) {
        return STATE_DEV_MODEL_SUBD_NOT_LOGIN;
    }

    memset(&request, 0, sizeof(dm_msg_request_t));
    request.reply_prefix = DM_URI_EXT_SESSION_PREFIX;
    request.reply_name = DM_URI_COMBINE_LOGOUT;
    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, request.product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_KEY, request.device_key);

    /* Get Params And Method */
    res = dm_msg_combine_logout(node->product_key, node->device_key, &request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Get Msg ID */
    request.msgid = iotx_report_id();

    /* Get Dev ID */
    request.devid = devid;

    /* Callback */
    request.callback = dm_client_combine_logout_reply;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_insert(request.msgid, request.devid, IOTX_DM_EVENT_COMBINE_LOGOUT_REPLY, NULL);
#endif
    /* Send Message To Cloud */
    res = dm_msg_request(DM_MSG_DEST_CLOUD, &request);
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (res != SUCCESS_RETURN) {
        dm_msg_cache_remove(request.msgid);
    } else {
        res = request.msgid;
    }
#endif
    DM_free(request.params);

    return res;
}

#ifdef DEVICE_MODEL_SUBDEV_OTA
int dm_mgr_upstream_thing_firmware_version_update(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;
    char *uri = NULL;
    dm_msg_request_t request;

    if (devid < 0 || payload == NULL || payload_len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    memset(&request, 0, sizeof(dm_msg_request_t));
    request.reply_prefix = DM_URI_OTA_DEVICE_INFORM;
    request.reply_name = NULL;
    memcpy(request.product_key, node->product_key, strlen(node->product_key));
    memcpy(request.device_key, node->device_key, strlen(node->device_key));

    /* Request URI */
    res = dm_utils_service_name(request.reply_prefix, request.reply_name,
                                request.product_key, request.device_key, &uri);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    res = dm_client_publish(uri, (unsigned char *)payload, strlen(payload), dm_client_thing_model_up_raw_reply);
    if (res < SUCCESS_RETURN) {
        DM_free(uri);
        return res;
    }

    DM_free(uri);
    return SUCCESS_RETURN;
}
#endif
#endif

int dm_mgr_upstream_thing_model_up_raw(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len)
{
    int res = 0, res1 = 0;
    dm_mgr_dev_node_t *node = NULL;
    char *uri = NULL;
    dm_msg_request_t request;

    if (devid < 0 || payload == NULL || payload_len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    memset(&request, 0, sizeof(dm_msg_request_t));
    request.service_prefix = DM_URI_SYS_PREFIX;
    request.service_name = DM_URI_THING_MODEL_UP_RAW;
    memcpy(request.product_key, node->product_key, strlen(node->product_key));
    memcpy(request.device_key, node->device_key, strlen(node->device_key));

    /* Request URI */
    res = dm_utils_service_name(request.service_prefix, request.service_name,
                                request.product_key, request.device_key, &uri);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    HEXDUMP_INFO(payload, payload_len);

    res = dm_client_publish(uri, (unsigned char *)payload, payload_len, dm_client_thing_model_up_raw_reply);

    DM_free(uri);
    return res;
}

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
static int _dm_mgr_upstream_request_assemble(_IN_ int msgid, _IN_ int devid, _IN_ const char *service_prefix,
        _IN_ const char *service_name,
        _IN_ char *params, _IN_ int params_len, _IN_ char *method, _OU_ dm_msg_request_t *request)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    request->msgid = msgid;
    request->devid = devid;
    request->service_prefix = service_prefix;
    request->service_name = service_name;
    memcpy(request->product_key, node->product_key, strlen(node->product_key));
    memcpy(request->device_key, node->device_key, strlen(node->device_key));
    request->params = params;
    request->params_len = params_len;
    request->method = method;

    return SUCCESS_RETURN;
}

int dm_mgr_upstream_common(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len, _IN_ char * method)
{
    int res = 0;
    int service_name_len = strlen(method) + 1;
    char* service_name = NULL;
    char* cptr = NULL;
    dm_msg_request_t request;
    int common_downstream_reply = 0;
    if (devid < 0 || payload == NULL || payload_len <= 0 || method == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    /* get service_name by method, replace . to / */
    service_name = DM_malloc(service_name_len);
    memset(service_name, 0, service_name_len);
    infra_str_replace(method, strlen(method), DM_ENOS_METHOD_DELIMITER, DM_URI_SERVICE_DELIMITER, &service_name);

    memset(&request, 0, sizeof(dm_msg_request_t));
    res = _dm_mgr_upstream_request_assemble(iotx_report_id(), devid, DM_URI_SYS_PREFIX, service_name,
                                            payload, payload_len, method, &request);
    if (res != SUCCESS_RETURN) {
    	DM_free(service_name);
        return res;
    }

    /* Callback */
    request.callback = dm_client_thing_reply;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    res = dm_opt_get(DM_OPT_DOWNSTREAM_THING_REPLY, &common_downstream_reply);
    if (res == SUCCESS_RETURN && common_downstream_reply) {
        dm_msg_cache_insert(request.msgid, request.devid, IOTX_DM_EVENT_THING_REPLY, NULL);
    }
#endif
    /* Send Message To Cloud */
    res = dm_msg_request(DM_MSG_DEST_ALL, &request);
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (res != SUCCESS_RETURN) {
        dm_msg_cache_remove(request.msgid);
    } else {
        res = request.msgid;
    }
#endif
	DM_free(service_name);

    return res;
}


#ifdef DEVICE_MEASUREPOINT_RESUME
int dm_mgr_upstream_thing_measurepoint_resume(int devid, char *payload, int payload_len)
{
    int res = 0;
    dm_msg_request_t request;
    int measurepoint_resume_reply = 0;
    if (devid < 0 || payload == NULL || payload_len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    memset(&request, 0, sizeof(dm_msg_request_t));
    res = _dm_mgr_upstream_request_assemble(iotx_report_id(), devid, DM_URI_SYS_PREFIX, DM_URI_THING_MEASUREPOINT_RESUME,
                                            payload, payload_len, "thing.measurepoint.resume", &request);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    /* Callback */
    request.callback = dm_client_thing_reply;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_insert(request.msgid, request.devid, IOTX_DM_EVENT_THING_REPLY, NULL);
#endif
    /* Send Message To Cloud */
    res = dm_msg_request(DM_MSG_DEST_ALL, &request);
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (res != SUCCESS_RETURN) {
        dm_msg_cache_remove(request.msgid);
    } else {
        res = request.msgid;
    }
#endif
    return res;
}
#endif /* #ifdef DEVICE_MEASUREPOINT_RESUME */


int dm_mgr_upstream_thing_event_post(_IN_ int devid, _IN_ char *identifier, _IN_ int identifier_len, _IN_ char *method,
                                     _IN_ char *payload, _IN_ int payload_len)
{
    int res = 0, service_name_len = 0;
    char *service_name = NULL;
    dm_msg_request_t request;
    int event_post_reply = 0;

    if (devid < 0 || identifier == NULL || identifier_len <= 0 ||
        method == NULL || payload == NULL || payload_len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    service_name_len = strlen(DM_URI_THING_EVENT_POST) + identifier_len + 1;
    service_name = DM_malloc(service_name_len);
    if (service_name == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(service_name, 0, service_name_len);
    HAL_Snprintf(service_name, service_name_len, DM_URI_THING_EVENT_POST, identifier_len, identifier);

    memset(&request, 0, sizeof(dm_msg_request_t));
    res = _dm_mgr_upstream_request_assemble(iotx_report_id(), devid, DM_URI_SYS_PREFIX, service_name,
                                            payload, payload_len, method, &request);
    if (res != SUCCESS_RETURN) {
    	DM_free(service_name);
        return res;
    }

    /* Callback */
    request.callback = dm_client_thing_reply;
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    res = dm_opt_get(DM_OPT_DOWNSTREAM_THING_REPLY, &event_post_reply);
    if (res == SUCCESS_RETURN && event_post_reply) {
        dm_msg_cache_insert(request.msgid, request.devid, IOTX_DM_EVENT_THING_REPLY, NULL);
    }
#endif
    /* Send Message To Cloud */
    res = dm_msg_request(DM_MSG_DEST_ALL, &request);
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    if (res != SUCCESS_RETURN) {
        dm_msg_cache_remove(request.msgid);
    } else {
        res = request.msgid;
    }
#endif
    DM_free(service_name);

    return res;
}


static int _dm_mgr_upstream_response_assemble(_IN_ int devid, _IN_ char *msgid, _IN_ int msgid_len,
        _IN_ const char *prefix,
        _IN_ const char *reply_name, _IN_ int code, _OU_ dm_msg_request_payload_t *request, _OU_ dm_msg_response_t *response)
{
    int res = 0;
    dm_mgr_dev_node_t *node = NULL;

    res = _dm_mgr_search_dev_by_devid(devid, &node);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    request->id.value = msgid;
    request->id.value_length = msgid_len;

    response->reply_prefix = DM_URI_SYS_PREFIX;
    response->reply_name = reply_name;
    memcpy(response->product_key, node->product_key, strlen(node->product_key));
    memcpy(response->device_key, node->device_key, strlen(node->device_key));
    response->code = (iotx_dm_error_code_t)code;

    return SUCCESS_RETURN;
}

int dm_mgr_upstream_thing_measurepoint_set_response(_IN_ int devid, _IN_ char *msgid, _IN_ int msgid_len,
        _IN_ iotx_dm_error_code_t code, _IN_ char *payload, _IN_ int payload_len, void *ctx)
{
    int res = 0, reply_name_len = 0;
    char *reply_name = NULL;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    if (devid < 0 || msgid == NULL || msgid_len <= 0 || payload == NULL || payload_len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    /* Service Name */
    reply_name_len = strlen(DM_URI_THING_MEASUREPOINT_SET_REPLY) + 1;
    reply_name = DM_malloc(reply_name_len);
    if (reply_name == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(reply_name, 0, reply_name_len);
    HAL_Snprintf(reply_name, reply_name_len, DM_URI_THING_MEASUREPOINT_SET_REPLY);

    res = _dm_mgr_upstream_response_assemble(devid, msgid, msgid_len, DM_URI_SYS_PREFIX, reply_name, code, &request,
            &response);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    if (ctx != NULL) {
        dm_msg_response(DM_MSG_DEST_LOCAL, &request, &response, payload, payload_len, ctx);
    } else {
        dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, payload, payload_len, ctx);
    }

    DM_free(reply_name);
    return SUCCESS_RETURN;
}

int dm_mgr_upstream_thing_service_invoke_response(_IN_ int devid, _IN_ char *msgid, _IN_ int msgid_len,
        _IN_ iotx_dm_error_code_t code,
        _IN_ char *identifier, _IN_ int identifier_len, _IN_ char *payload, _IN_ int payload_len, void *ctx)
{
    int res = 0, service_name_len = 0;
    char *service_name = NULL;
    dm_msg_request_payload_t request;
    dm_msg_response_t response;

    memset(&request, 0, sizeof(dm_msg_request_payload_t));
    memset(&response, 0, sizeof(dm_msg_response_t));

    if (devid < 0 || msgid == NULL || msgid_len <= 0 || identifier == NULL || identifier_len <= 0 ||
        payload == NULL || payload_len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    /* Service Name */
    service_name_len = strlen(DM_URI_THING_SERVICE_INVOKE_REPLY) + identifier_len + 1;
    service_name = DM_malloc(service_name_len);
    if (service_name == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(service_name, 0, service_name_len);
    HAL_Snprintf(service_name, service_name_len, DM_URI_THING_SERVICE_INVOKE_REPLY, identifier_len, identifier);

    res = _dm_mgr_upstream_response_assemble(devid, msgid, msgid_len, DM_URI_SYS_PREFIX, service_name, code, &request,
            &response);
    if (res != SUCCESS_RETURN) {
        return res;
    }

    if (ctx != NULL) {
        dm_msg_response(DM_MSG_DEST_LOCAL, &request, &response, payload, payload_len, ctx);
    } else {
        dm_msg_response(DM_MSG_DEST_CLOUD, &request, &response, payload, payload_len, ctx);
    }

    DM_free(service_name);
    return SUCCESS_RETURN;
}

#endif
