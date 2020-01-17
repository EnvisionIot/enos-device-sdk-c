LIBA_TARGET     := libiot_dynamic_activate.a

HDR_REFS        := src/infra

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_tls

TARGET          := dynamic-activate-example

LIB_SRCS_EXCLUDE      := examples/dynamic_activate_example.c
SRCS_dynamic-activate-example   += examples/dynamic_activate_example.c