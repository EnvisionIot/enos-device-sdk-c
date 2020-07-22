#ifndef _DM_CLIENT_H_
#define _DM_CLIENT_H_

typedef struct {
    const char *uri_name;
    const char *uri_prefix;
    int dev_type;
    void *callback;
} dm_client_uri_map_t;

void dm_client_event_handle(int fd, iotx_cm_event_msg_t *event, void *context);

int dm_client_subscribe_all(int devid, char product_key[IOTX_PRODUCT_KEY_LEN + 1], char device_key[IOTX_DEVICE_KEY_LEN + 1],
                            int dev_type);

void dm_client_thing_model_down_raw(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context);
void dm_client_thing_model_up_raw_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                        void *context);

#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
void dm_client_thing_measurepoint_desired_get_reply(int fd, const char *topic, const char *payload,
        unsigned int payload_len, void *context);
void dm_client_thing_measurepoint_desired_delete_reply(int fd, const char *topic, const char *payload,
        unsigned int payload_len, void *context);
void dm_client_thing_measurepoint_set(int fd, const char *topic, const char *payload, unsigned int payload_len,
        void *context);
void dm_client_thing_service_measurepoint_get(int fd, const char *topic, const char *payload, unsigned int payload_len,
        void *context);
void dm_client_thing_service_measurepoint_post(int fd, const char *topic, const char *payload, unsigned int payload_len,
        void *context);
void dm_client_thing_event_measurepoint_post_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
        void *context);
void dm_client_thing_service_invoke(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                     void *context);
void dm_client_thing_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                      void *context);
#endif

#ifdef DEVICE_MODEL_GATEWAY
int dm_client_subdev_unsubscribe(char product_key[IOTX_PRODUCT_KEY_LEN + 1],
                                 char device_key[IOTX_DEVICE_KEY_LEN + 1]);
void dm_client_thing_topo_add_notify(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                     void *context);
void dm_client_thing_disable(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context);
void dm_client_thing_enable(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context);
void dm_client_thing_delete(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context);
void dm_client_combine_disable(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context);
void dm_client_combine_enable(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context);
void dm_client_combine_delete(int fd, const char *topic, const char *payload, unsigned int payload_len, void *context);
void dm_client_thing_gateway_permit(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context);
void dm_client_thing_device_register_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                        void *context);
void dm_client_thing_proxy_product_register_reply(int fd, const char *topic, const char *payload,
        unsigned int payload_len,
        void *context);
void dm_client_thing_sub_unregister_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
        void *context);
void dm_client_thing_topo_add_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context);
void dm_client_thing_topo_delete_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                       void *context);
void dm_client_thing_topo_get_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context);
void dm_client_thing_list_found_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                      void *context);
void dm_client_combine_login_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                   void *context);
void dm_client_combine_login_batch_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                   void *context);
void dm_client_combine_logout_reply(int fd, const char *topic, const char *payload, unsigned int payload_len,
                                    void *context);
#endif
#endif
