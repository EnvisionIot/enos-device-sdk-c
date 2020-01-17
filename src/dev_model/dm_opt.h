/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */

#ifdef DEVICE_MODEL_ENABLED
#ifndef _DM_OPT_H
#define _DM_OPT_H

typedef enum {
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
    DM_OPT_DOWNSTREAM_THING_REPLY,
    DM_OPT_UPSTREAM_MEASUREPOINT_SET_AUTO_REPLY,
#endif
    DM_OPT_MAX
} dm_opt_t;

typedef struct {
    int thing_reply_opt;
    int measurepoint_set_auto_reply_opt;
} dm_opt_ctx;

int dm_opt_set(dm_opt_t opt, void *data);
int dm_opt_get(dm_opt_t opt, void *data);

#endif
#endif
