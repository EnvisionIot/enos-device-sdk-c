LIBA_TARGET     := libiot_enos.a

HDR_REFS        += src/infra
HDR_REFS		+= src/mqtt
HDR_REFS		+= src/dev_sign

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_tls

LIB_SRCS_PATTERN     	:= *.c client/*.c

LIB_SRCS_EXCLUDE     	  := examples/*.c
SRCS_enos-example-solo    := examples/enos_example_solo.c examples/cJSON.c

LIB_SRCS_EXCLUDE          += examples/enos_example_dynamic_activate.c examples/cJSON.c
SRCS_enos-example-dynamic-activate    := examples/enos_example_dynamic_activate.c examples/cJSON.c

LIB_SRCS_EXCLUDE          += examples/enos_example_device_attribute.c examples/cJSON.c
SRCS_enos-example-device-attribute    := examples/enos_example_device_attribute.c examples/cJSON.c

LIB_SRCS_EXCLUDE          += examples/enos_example_device_tag.c examples/cJSON.c
SRCS_enos-example-device-tag    := examples/enos_example_device_tag.c examples/cJSON.c

LIB_SRCS_EXCLUDE          += examples/enos_example_device_measurepoint.c examples/cJSON.c
SRCS_enos-example-device-measurepoint    := examples/enos_example_device_measurepoint.c examples/cJSON.c

LIB_SRCS_EXCLUDE          += examples/enos_example_device_event.c examples/cJSON.c
SRCS_enos-example-device-event    := examples/enos_example_device_event.c examples/cJSON.c

LIB_SRCS_EXCLUDE     	  := examples/*.c
SRCS_enos-example-command    := examples/enos_example_command.c examples/cJSON.c

LIB_SRCS_EXCLUDE          += examples/enos_example_device_login_by_ssl.c examples/cJSON.c
SRCS_enos-example-device-login-by-ssl    := examples/enos_example_device_login_by_ssl.c examples/cJSON.c

LIB_SRCS_EXCLUDE          += examples/enos_example_raw.c examples/cJSON.c
SRCS_enos-example-raw    := examples/enos_example_raw.c examples/cJSON.c


#LIB_SRCS_EXCLUDE          += examples/enos_example_gateway.c examples/cJSON.c
#SRCS_enos-example-gateway := examples/enos_example_gateway.c examples/cJSON.c

$(call Append_Conditional, TARGET, enos-example-solo, DEVICE_MODEL_ENABLED, BUILD_AOS NO_EXECUTABLES)
$(call Append_Conditional, TARGET, enos-example-dynamic-activate, DYNAMIC_ACTIVATE, BUILD_AOS NO_EXECUTABLES)
$(call Append_Conditional, TARGET, enos-example-device-attribute, DEVICE_MODEL_ENABLED, BUILD_AOS NO_EXECUTABLES)
$(call Append_Conditional, TARGET, enos-example-device-tag, DEVICE_MODEL_ENABLED, BUILD_AOS NO_EXECUTABLES)
$(call Append_Conditional, TARGET, enos-example-device-measurepoint, DEVICE_MODEL_ENABLED, BUILD_AOS NO_EXECUTABLES)
$(call Append_Conditional, TARGET, enos-example-device-event, DEVICE_MODEL_ENABLED, BUILD_AOS NO_EXECUTABLES)
$(call Append_Conditional, TARGET, enos-example-command, DEVICE_MODEL_ENABLED, BUILD_AOS NO_EXECUTABLES)
$(call Append_Conditional, TARGET, enos-example-device-login-by-ssl, DEVICE_MODEL_ENABLED, BUILD_AOS NO_EXECUTABLES)
$(call Append_Conditional, TARGET, enos-example-raw, DEVICE_MODEL_ENABLED, BUILD_AOS NO_EXECUTABLES)



#$(call Append_Conditional, TARGET, enos-example-gateway, DEVICE_MODEL_GATEWAY, BUILD_AOS NO_EXECUTABLES)
