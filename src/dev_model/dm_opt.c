/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */
#include "iotx_dm_internal.h"

#ifdef DEVICE_MODEL_ENABLED

static dm_opt_ctx g_dm_opt = {
    0, /* disable receive thing reply */
    0  /* disable send measurepoint set reply automatically */
};

int dm_opt_set(dm_opt_t opt, void *data)
{
    int res = SUCCESS_RETURN;

    if (data == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    switch (opt) {
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
        case DM_OPT_DOWNSTREAM_THING_REPLY: {
            int opt = *(int *)(data);
            g_dm_opt.thing_reply_opt = opt;
        }
        break;
        case DM_OPT_UPSTREAM_MEASUREPOINT_SET_AUTO_REPLY: {
            int opt = *(int *)(data);
            g_dm_opt.measurepoint_set_auto_reply_opt = opt;
        }
        break;
#endif
        default: {
            res = STATE_USER_INPUT_INVALID;
        }
        break;
    }

    return res;
}

int dm_opt_get(dm_opt_t opt, void *data)
{
    int res = SUCCESS_RETURN;

    if (data == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    switch (opt) {
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
        case DM_OPT_DOWNSTREAM_THING_REPLY: {
            *(int *)(data) = g_dm_opt.thing_reply_opt;
        }
        break;
        case DM_OPT_UPSTREAM_MEASUREPOINT_SET_AUTO_REPLY: {
            *(int *)(data) = g_dm_opt.measurepoint_set_auto_reply_opt;
        }
        break;
#endif
        default: {
            res = STATE_DEV_MODEL_INVALID_DM_OPTION;
        }
        break;
    }

    return res;
}
#endif
