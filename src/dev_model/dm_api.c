/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */

#include "iotx_dm_internal.h"

static dm_api_ctx_t g_dm_api_ctx;

static dm_api_ctx_t *_dm_api_get_ctx(void)
{
    return &g_dm_api_ctx;
}

static void _dm_api_lock(void)
{
    dm_api_ctx_t *ctx = _dm_api_get_ctx();
    if (ctx->mutex)
    {
        HAL_MutexLock(ctx->mutex);
    }
}

static void _dm_api_unlock(void)
{
    dm_api_ctx_t *ctx = _dm_api_get_ctx();
    if (ctx->mutex)
    {
        HAL_MutexUnlock(ctx->mutex);
    }
}

int iotx_dm_open(void)
{
    int res = 0;
    dm_api_ctx_t *ctx = _dm_api_get_ctx();
    memset(ctx, 0, sizeof(dm_api_ctx_t));

    /* DM Mutex Create*/
    ctx->mutex = HAL_MutexCreate();
    if (ctx->mutex == NULL)
    {
        return STATE_SYS_DEPEND_MUTEX_CREATE;
    }

#if defined(OTA_ENABLED) && !defined(BUILD_AOS)
    /* DM OTA Module Init */
    res = dm_ota_init();
    if (res != SUCCESS_RETURN)
    {
        goto ERROR;
    }
#endif

#if !defined(DM_MESSAGE_CACHE_DISABLED)
    /* DM Message Cache Init */
    res = dm_msg_cache_init();
    if (res != SUCCESS_RETURN)
    {
        goto ERROR;
    }
#endif
    /* DM Cloud Message Parse And Assemble Module Init */
    res = dm_msg_init();
    if (res != SUCCESS_RETURN)
    {
        goto ERROR;
    }

    /* DM IPC Module Init */
    res = dm_ipc_init(CONFIG_DISPATCH_QUEUE_MAXLEN);
    if (res != SUCCESS_RETURN)
    {
        goto ERROR;
    }

    /* DM Manager Module Init */
    res = dm_mgr_init();
    if (res != SUCCESS_RETURN)
    {
        goto ERROR;
    }

    /* Open Cloud Connection */
    res = dm_client_open();
    if (res < SUCCESS_RETURN)
    {
        goto ERROR;
    }

    return SUCCESS_RETURN;

ERROR:
    dm_client_close();
    dm_mgr_deinit();
    dm_ipc_deinit();
    dm_msg_deinit();
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_deinit();
#endif
#if defined(OTA_ENABLED) && !defined(BUILD_AOS)
    dm_ota_deinit();
#endif

    if (ctx->mutex)
    {
        HAL_MutexDestroy(ctx->mutex);
    }
    return res;
}

int iotx_dm_connect(_IN_ iotx_dm_init_params_t *init_params)
{
    int res = 0;
    dm_api_ctx_t *ctx = _dm_api_get_ctx();

    /* DM Event Callback */
    if (init_params->event_callback != NULL)
    {
        ctx->event_callback = init_params->event_callback;
    }

    res = dm_client_connect(IOTX_DM_CLIENT_CONNECT_TIMEOUT_MS);
    if (res != SUCCESS_RETURN)
    {
        return res;
    }

#if defined(OTA_ENABLED) && !defined(BUILD_AOS)
    /* DM OTA Module Init */
    res = dm_ota_sub();
    if (res == SUCCESS_RETURN)
    {
    }
#endif

    return SUCCESS_RETURN;
}

int iotx_dm_subscribe(_IN_ int devid)
{
    int res = 0, dev_type = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};
    char device_secret[IOTX_DEVICE_SECRET_LEN + 1] = {0};

    if (devid < 0)
    {
        return STATE_USER_INPUT_INVALID;
    }

    _dm_api_lock();
    res = dm_mgr_search_device_by_devid(devid, product_key, device_key, device_secret);
    if (res < SUCCESS_RETURN)
    {
        _dm_api_unlock();
        return res;
    }

    res = dm_mgr_get_dev_type(devid, &dev_type);
    if (res < SUCCESS_RETURN)
    {
        _dm_api_unlock();
        return res;
    }

    res = dm_client_subscribe_all(devid, product_key, device_key, dev_type);
    if (res < SUCCESS_RETURN)
    {
        _dm_api_unlock();
        return res;
    }
    _dm_api_unlock();

    return SUCCESS_RETURN;
}

int iotx_dm_close(void)
{
    dm_api_ctx_t *ctx = _dm_api_get_ctx();

    dm_client_close();
    dm_mgr_deinit();
    dm_ipc_deinit();
    dm_msg_deinit();
#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_deinit();
#endif
#if defined(OTA_ENABLED) && !defined(BUILD_AOS)
    dm_ota_deinit();
#endif

    if (ctx->mutex)
    {
        HAL_MutexDestroy(ctx->mutex);
    }

    return SUCCESS_RETURN;
}

int iotx_dm_yield(int timeout_ms)
{
    if (timeout_ms <= 0)
    {
        return STATE_USER_INPUT_INVALID;
    }

    dm_client_yield(timeout_ms);

    return STATE_SUCCESS;
}

void iotx_dm_dispatch(void)
{
    int count = 0;
    void *data = NULL;
    dm_api_ctx_t *ctx = _dm_api_get_ctx();

#if !defined(DM_MESSAGE_CACHE_DISABLED)
    dm_msg_cache_tick();
#endif

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
    iotx_enos_service_list_overtime_handle();
#endif

    while (CONFIG_DISPATCH_QUEUE_MAXLEN == 0 || count++ < CONFIG_DISPATCH_QUEUE_MAXLEN)
    {
        if (dm_ipc_msg_next(&data) == SUCCESS_RETURN)
        {
            dm_ipc_msg_t *msg = (dm_ipc_msg_t *)data;

            if (ctx->event_callback)
            {
                ctx->event_callback(msg->type, msg->data);
            }

            if (msg->data)
            {
                DM_free(msg->data);
            }
            DM_free(msg);
            data = NULL;
        }
        else
        {
            break;
        }
    }
}

void *g_user_topic_callback = NULL;

int iotx_dm_subscribe_user_topic(char *topic, void *user_callback)
{
    char *uri = NULL;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};
    int res = 0;

    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_KEY, device_key);

    res = dm_utils_service_name((char *)DM_URI_SYS_PREFIX, topic,
                                product_key, device_key, &uri);
    g_user_topic_callback = user_callback;

    if (res < SUCCESS_RETURN)
    {
        return res;
    }

    {
        int retry_cnt = IOTX_DM_CLIENT_SUB_RETRY_MAX_COUNTS;
        int local_sub = 0;
        do
        {
            res = dm_client_subscribe(uri, dm_client_user_sub_request, &local_sub);
        } while (res < SUCCESS_RETURN && --retry_cnt);
        DM_free(uri);
    }

    return res;
}

int iotx_dm_post_rawdata(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len)
{
    int res = 0;

    _dm_api_lock();
    res = dm_mgr_upstream_thing_model_up_raw(devid, payload, payload_len);
    _dm_api_unlock();

    return res;
}

int iotx_dm_set_opt(int opt, void *data)
{
    return dm_opt_set((dm_opt_t)opt, data);
}

int iotx_dm_get_opt(int opt, void *data)
{
    if (data == NULL)
    {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    return dm_opt_get((dm_opt_t)opt, data);
}

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
int iotx_dm_upstream_common(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len,
                            _IN_ char *method)
{
    int res = 0;
    _dm_api_lock();
    res = dm_mgr_upstream_common(devid, payload, payload_len, method);
    _dm_api_unlock();
    return res;
}

#ifdef DEVICE_MEASUREPOINT_RESUME
int iotx_dm_resume_measurepoint(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len)
{
    int res = 0;

    _dm_api_lock();
    res = dm_mgr_upstream_thing_measurepoint_resume(devid, payload, payload_len);
    _dm_api_unlock();

    return res;
}
#endif /* #ifdef DEVICE_MEASUREPOINT_RESUME */

#endif

int iotx_dm_post_event(_IN_ int devid, _IN_ char *identifier, _IN_ int identifier_len, _IN_ char *payload,
                       _IN_ int payload_len)
{
    int res = 0, method_len = 0;
    const char *method_fmt = "thing.event.%.*s.post";
    char *method = NULL;

    if (devid < 0 || identifier == NULL || identifier_len == 0 || payload == NULL || payload_len <= 0)
    {
        return STATE_USER_INPUT_INVALID;
    }

    _dm_api_lock();

    method_len = strlen(method_fmt) + strlen(identifier) + 1;
    method = DM_malloc(method_len);
    if (method == NULL)
    {
        _dm_api_unlock();
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(method, 0, method_len);
    HAL_Snprintf(method, method_len, method_fmt, identifier_len, identifier);

    res = dm_mgr_upstream_thing_event_post(devid, identifier, identifier_len, method, payload, payload_len);
    DM_free(method);
    _dm_api_unlock();

    return res;
}

int iotx_dm_send_measurepoint_set_response(_IN_ int devid, _IN_ char *msgid, _IN_ int msgid_len, _IN_ iotx_dm_error_code_t code,
                                   _IN_ char *payload, _IN_ int payload_len, void *ctx)
{
    int res = 0;

    _dm_api_lock();
    res = dm_mgr_upstream_thing_measurepoint_set_response(devid, msgid, msgid_len, code, payload, payload_len, ctx);
    _dm_api_unlock();

    return res;
}

int iotx_dm_send_service_invoke_response(_IN_ int devid, _IN_ char *msgid, _IN_ int msgid_len, _IN_ iotx_dm_error_code_t code,
                                  _IN_ char *identifier,
                                  _IN_ int identifier_len, _IN_ char *payload, _IN_ int payload_len, void *ctx)
{
    int res = 0;

    _dm_api_lock();
    res = dm_mgr_upstream_thing_service_invoke_response(devid, msgid, msgid_len, code, identifier, identifier_len, payload,
                                                 payload_len, ctx);
    _dm_api_unlock();

    return res;
}

#ifdef DEVICE_MODEL_GATEWAY
int iotx_dm_query_topo_list(void)
{
    int res = 0;

    _dm_api_lock();
    res = dm_mgr_upstream_thing_topo_get();
    _dm_api_unlock();

    return res;
}

int iotx_dm_subdev_query(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                         _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                         _OU_ int *devid)
{
    int res = 0;

    _dm_api_lock();
    res = dm_mgr_device_query(product_key, device_key, devid);
    _dm_api_unlock();

    return res;
}

int iotx_dm_subdev_create(_IN_ char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                          _IN_ char product_secret[IOTX_PRODUCT_SECRET_LEN + 1],
                          _IN_ char device_key[IOTX_DEVICE_KEY_LEN + 1],
                          _IN_ char device_secret[IOTX_DEVICE_SECRET_LEN + 1], _OU_ int *devid)
{
    int res = 0;

    _dm_api_lock();
    res = dm_mgr_device_create(IOTX_DM_DEVICE_SUBDEV, product_key, product_secret, device_key, device_secret, devid);
    _dm_api_unlock();

    return res;
}

int iotx_dm_subdev_destroy(_IN_ int devid)
{
    int res = 0;

    _dm_api_lock();
    res = dm_mgr_device_destroy(devid);
    _dm_api_unlock();

    return SUCCESS_RETURN;
}

int iotx_dm_subdev_number(void)
{
    int number = 0;

    _dm_api_lock();
    number = dm_mgr_device_number();
    _dm_api_unlock();

    return number;
}

int iotx_dm_subdev_register(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *search_node = NULL;

    _dm_api_lock();
    res = dm_mgr_search_device_node_by_devid(devid, (void **)&search_node);
    if (res != SUCCESS_RETURN)
    {
        _dm_api_unlock();
        return res;
    }

    if ((strlen(search_node->device_secret) > 0) && (strlen(search_node->device_secret) < IOTX_DEVICE_SECRET_LEN + 1))
    {
        _dm_api_unlock();
        return SUCCESS_RETURN;
    }

    res = dm_mgr_upstream_thing_sub_register(devid);

    _dm_api_unlock();
    return res;
}

int iotx_dm_subdev_proxy_register(_IN_ int devid)
{
    int res = 0;
    dm_mgr_dev_node_t *search_node = NULL;

    if (devid < 0)
    {
        return STATE_USER_INPUT_INVALID;
    }

    _dm_api_lock();
    res = dm_mgr_search_device_node_by_devid(devid, (void **)&search_node);
    if (res != SUCCESS_RETURN)
    {
        _dm_api_unlock();
        return STATE_DEV_MODEL_DEVICE_NOT_FOUND;
    }

    if ((strlen(search_node->device_secret) > 0) && (strlen(search_node->device_secret) < IOTX_DEVICE_SECRET_LEN + 1))
    {
        _dm_api_unlock();
        return SUCCESS_RETURN;
    }

    res = dm_mgr_upstream_thing_proxy_product_register(devid);

    _dm_api_unlock();
    return res;
}

int iotx_dm_subdev_unregister(_IN_ int devid)
{
    int res = 0;

    if (devid < 0)
    {
        return STATE_USER_INPUT_INVALID;
    }

    _dm_api_lock();

    res = dm_mgr_upstream_thing_sub_unregister(devid);

    _dm_api_unlock();
    return res;
}

int iotx_dm_subdev_topo_add(_IN_ int devid)
{
    int res = 0;

    if (devid < 0)
    {
        return STATE_USER_INPUT_INVALID;
    }

    _dm_api_lock();

    res = dm_mgr_upstream_thing_topo_add(devid);

    _dm_api_unlock();
    return res;
}

int iotx_dm_subdev_topo_del(_IN_ int devid)
{
    int res = 0;

    if (devid < 0)
    {
        return STATE_USER_INPUT_INVALID;
    }

    _dm_api_lock();

    res = dm_mgr_upstream_thing_topo_delete(devid);

    _dm_api_unlock();
    return res;
}

int iotx_dm_subdev_login(_IN_ int devid)
{
    int res = 0;

    if (devid < 0)
    {
        return STATE_USER_INPUT_INVALID;
    }

    _dm_api_lock();

    res = dm_mgr_upstream_combine_login(devid);

    _dm_api_unlock();
    return res;
}

int iotx_dm_subdev_logout(_IN_ int devid)
{
    int res = 0;

    if (devid < 0)
    {
        return STATE_USER_INPUT_INVALID;
    }

    _dm_api_lock();

    res = dm_mgr_upstream_combine_logout(devid);

    _dm_api_unlock();
    return res;
}

int iotx_dm_get_device_type(_IN_ int devid, _OU_ int *type)
{
    int res = 0;

    if (devid < 0 || type == NULL)
    {
        return STATE_USER_INPUT_INVALID;
    }

    _dm_api_lock();
    res = dm_mgr_get_dev_type(devid, type);
    _dm_api_unlock();

    return res;
}

int iotx_dm_get_device_avail_status(_IN_ int devid, _OU_ iotx_dm_dev_avail_t *status)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_key[IOTX_DEVICE_KEY_LEN + 1] = {0};
    char device_secret[IOTX_DEVICE_SECRET_LEN + 1] = {0};

    if (devid < 0 || status == NULL)
    {
        return STATE_USER_INPUT_INVALID;
    }

    _dm_api_lock();
    res = dm_mgr_search_device_by_devid(devid, product_key, device_key, device_secret);
    if (res != SUCCESS_RETURN)
    {
        _dm_api_unlock();
        return res;
    }

    res = dm_mgr_get_dev_avail(product_key, device_key, status);
    if (res != SUCCESS_RETURN)
    {
        _dm_api_unlock();
        return res;
    }

    _dm_api_unlock();
    return SUCCESS_RETURN;
}

int iotx_dm_get_device_status(_IN_ int devid, _OU_ iotx_dm_dev_status_t *status)
{
    int res = 0;

    if (devid < 0 || status == NULL)
    {
        return STATE_USER_INPUT_INVALID;
    }

    _dm_api_lock();
    res = dm_mgr_get_dev_status(devid, status);
    _dm_api_unlock();

    return res;
}

#ifdef DEVICE_MODEL_SUBDEV_OTA
int iotx_dm_firmware_version_update(_IN_ int devid, _IN_ char *payload, _IN_ int payload_len)
{
    int res = 0;

    _dm_api_lock();
    res = dm_mgr_upstream_thing_firmware_version_update(devid, payload, payload_len);
    _dm_api_unlock();

    return res;
}

int iotx_dm_send_firmware_version(int devid, const char *version)
{
    char msg[FIRMWARE_VERSION_MSG_LEN] = {0};
    int msg_len = 0;
    /* firmware report message json data generate */
    int ret = HAL_Snprintf(msg,
                           FIRMWARE_VERSION_MSG_LEN,
                           "{\"id\":\"%d\",\"params\":{\"version\":\"%s\"}}",
                           iotx_report_id(),
                           version);
    if (ret <= 0)
    {
        return STATE_SYS_DEPEND_SNPRINTF;
    }

    msg_len = strlen(msg);

    ret = iotx_dm_firmware_version_update(devid, msg, msg_len);
    return ret;
}

int iotx_dm_ota_switch_device(_IN_ int devid)
{
    return dm_ota_switch_device(devid);
}
#endif
#endif
