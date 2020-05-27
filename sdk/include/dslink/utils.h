#ifndef SDK_DSLINK_C_UTILS_H
#define SDK_DSLINK_C_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define DSLINK_CHECKED_EXEC(func, val) \
    if (val) func(val)

const char *dslink_strcasestr(const char *haystack, const char *needle);
int dslink_strcasecmp(const char *a, const char *b);
char *dslink_strdup(const char *str);
char *dslink_strdupl(const char *str, size_t len);
int dslink_str_starts_with(const char *a, const char *b);
char *dslink_str_replace_all(const char *haystack,
                             const char *needle,
                             const char *replacement);
char *dslink_str_escape(const char *data);
char *dslink_str_unescape(const char *data);

size_t dslink_create_ts(char *buf, size_t bufLen);

int dslink_sleep(long ms);

const char* dslink_checkIpv4Address(const char* address);
const char* dslink_checkIpv6Address(const char* address);
int dslink_isipv6address(const char* host);
int is_start_with(const char *str, char *start);
int is_end_with(const char *str, char *end);
#ifdef __cplusplus
}
#endif

#endif // SDK_DSLINK_C_UTILS_H
