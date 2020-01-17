#ifndef _DYNAMIC_ACTIVATE_INTERNAL_H_
    #define _DYNAMIC_ACTIVATE_INTERNAL_H_

    #include "wrappers.h"

    #ifdef INFRA_LOG
        #include "infra_log.h"
        #define dynamic_activate_info(...)                log_info("dynamic_activate", __VA_ARGS__)
        #define dynamic_activate_err(...)                 log_err("dynamic_activate", __VA_ARGS__)
        #define dynamic_activate_dbg(...)                 log_debug("dynamic_activate", __VA_ARGS__)
    #else
        #define dynamic_activate_info(...)                do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
        #define dynamic_activate_err(...)                 do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
        #define dynamic_activate_dbg(...)                 do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
    #endif

    #ifdef INFRA_MEM_STATS
        #include "infra_mem_stats.h"
        #define dynamic_activate_malloc(size)             LITE_malloc(size, MEM_MAGIC, "dynamic_activate")
        #define dynamic_activate_free(ptr)                LITE_free(ptr)
    #else
        #define dynamic_activate_malloc(size)             HAL_Malloc(size)
        #define dynamic_activate_free(ptr)                {HAL_Free((void *)ptr);ptr = NULL;}
    #endif

#endif

#ifdef DYNAMIC_ACTIVATE
#include "mqtt_api.h"
void *wrapper_mqtt_init(iotx_mqtt_param_t *mqtt_params);
int wrapper_mqtt_connect(void *client);
int wrapper_mqtt_yield(void *client, int timeout_ms);
int wrapper_mqtt_subscribe(void *client,
                           const char *topicFilter,
                           iotx_mqtt_qos_t qos,
                           iotx_mqtt_event_handle_func_fpt topic_handle_func,
                           void *pcontext);
int wrapper_mqtt_release(void **pclient);
#endif
