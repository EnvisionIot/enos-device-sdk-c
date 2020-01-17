#ifndef _DYNAMIC_ACTIVATE_API_H_
#define _DYNAMIC_ACTIVATE_API_H_

#include "infra_types.h"
#include "infra_defs.h"

#define MQTT_DYNAMIC_ACTIVATE_TIMEOUT_MS (15 * 1000)

int32_t IOT_Dynamic_Activate(iotx_mqtt_region_types_t region, iotx_dev_meta_info_t *meta);

#endif

