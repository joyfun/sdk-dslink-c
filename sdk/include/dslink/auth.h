#ifndef BROKER_NET_AUTH_H
#define BROKER_NET_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct AccessToken {
  char *acc_token;
  char *token_type;
  int *expires_in;
  char *refresh_token;
  int *expires_time;
} AccessToken;


#ifdef __cplusplus
}
#endif

#endif // BROKER_NET_AUTH_H