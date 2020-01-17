/*
 * Copyright (C) 2015-2019 Envision Digital Limited
 */



#ifndef _IOT_DM_API_H_
#define _IOT_DM_API_H_

typedef struct {
    void *mutex;
    void *cloud_connectivity;
    void *local_connectivity;
    iotx_dm_event_callback event_callback;
} dm_api_ctx_t;

#endif
