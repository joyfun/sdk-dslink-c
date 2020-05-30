#ifndef SOUTH_CLIENT_H
#define SOUTH_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

int mqtt_connection (struct mosquitto* m_pMqtt,char* host,int port,char* user,char* pass,char* client_id,int keepalive);


#ifdef __cplusplus
}
#endif

#endif // SOUTH_CLIENT_H