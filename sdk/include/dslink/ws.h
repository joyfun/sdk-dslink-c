#ifndef SDK_DSLINK_C_WS_H
#define SDK_DSLINK_C_WS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mbedtls/ecdh.h>
#include <wslay/wslay.h>
#include <jansson.h>

#include "dslink/dslink.h"
#include "dslink/socket.h"
#include "dslink/err.h"
#include "dslink/url.h"

int dslink_handshake_connect_ws(Url *url,
                                mbedtls_ecdh_context *key,
                                const char *uri,
                                const char *tempKey,
                                const char *salt,
                                const char *dsId,
                                Socket **sock);
void dslink_handshake_handle_ws(DSLink *link);

int dslink_ws_send_obj(wslay_event_context_ptr ctx, json_t *obj);
int dslink_ws_send(wslay_event_context_ptr ctx,
                   const char *data);

#ifdef __cplusplus
}
#endif

#endif // SDK_DSLINK_C_WS_H
