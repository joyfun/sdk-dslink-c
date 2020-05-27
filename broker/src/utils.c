#include <dslink/utils.h>
#include "broker/net/ws.h"
#include "broker/utils.h"

#include <string.h>

void broker_free_handle(uv_handle_t *handle) {
    dslink_free(handle);
}

void broker_utils_send_closed_resp(RemoteDSLink *link, json_t *req, const char* errorType) {
    if (!link || !req) {
        return;
    }
    json_t *top = json_object();
    json_t *resps = json_array();
    json_object_set_new_nocheck(top, "responses", resps);
    json_t *resp = json_object();
    json_array_append_new(resps, resp);


    json_t *rid;
    if (json_is_object(req)) {
        rid = json_object_get(req, "rid");
        json_object_set_nocheck(resp, "rid", rid);
    } else if (json_is_integer(req)) {
        json_object_set_nocheck(resp, "rid", req);
    }


    json_object_set_new_nocheck(resp, "stream",
                                json_string_nocheck("closed"));
    if (errorType) {
        json_t * errorObject = json_object();
        json_object_set_new_nocheck(errorObject, "type", json_string_nocheck(errorType));
        json_object_set_new_nocheck(resp, "error", errorObject);
    }

    broker_ws_send_obj(link, top);
    json_decref(top);
}

void broker_utils_send_static_list_resp(RemoteDSLink *link, json_t *req) {
    if (!link || !req) {
        return;
    }
    json_t *top = json_object();
    json_t *resps = json_array();
    json_object_set_new_nocheck(top, "responses", resps);
    json_t *resp = json_object();
    json_array_append_new(resps, resp);


    json_t *rid;
    if (json_is_object(req)) {
        rid = json_object_get(req, "rid");
        json_object_set_nocheck(resp, "rid", rid);
    }

    json_t *updates = json_array();
    json_object_set_new_nocheck(resp, "updates", updates);

    json_t *update = json_array();
    json_array_append_new(updates,update);

    json_array_append_new(update, json_string_nocheck("$is"));
    json_array_append_new(update, json_string_nocheck("static"));

    json_object_set_new_nocheck(resp, "stream",
                                json_string_nocheck("closed"));


    broker_ws_send_obj(link, top);
    json_decref(top);
}

void broker_utils_send_disconnected_list_resp(RemoteDSLink *link, json_t *req) {
    if (!link || !req) {
        return;
    }
    json_t *top = json_object();
    json_t *resps = json_array();
    json_object_set_new_nocheck(top, "responses", resps);
    json_t *resp = json_object();
    json_array_append_new(resps, resp);


    json_t *rid;
    if (json_is_object(req)) {
        rid = json_object_get(req, "rid");
        json_object_set_nocheck(resp, "rid", rid);
    }

    json_t *updates = json_array();
    json_object_set_new_nocheck(resp, "updates", updates);

    json_t *update = json_array();
    json_array_append_new(updates,update);

    json_array_append_new(update, json_string_nocheck("$disconnectedTs"));

    char disconnectedTs[32];
    dslink_create_ts(disconnectedTs, 32);
    json_array_append_new(update, json_string_nocheck(disconnectedTs));

    json_object_set_new_nocheck(resp, "stream",
                                json_string_nocheck("closed"));


    broker_ws_send_obj(link, top);
    json_decref(top);
}

const char* setHostFrom(const char* protocol,const char* address, const char* port)
{
    char* host = NULL;

    if(dslink_isipv6address(address)) {
        const char* temp = dslink_checkIpv6Address(address);
        int len = strlen(temp)+strlen(port)+16+1;
        host = dslink_malloc(len);
        snprintf(host, len, "%s://[%s]:%s/conn", protocol, temp, port);
    } else {
        const char* temp = dslink_checkIpv4Address(address);
        int len = strlen(temp)+strlen(port)+16+1;
        host = dslink_malloc(len);
        snprintf(host, len, "%s://%s:%s/conn", protocol, temp, port);
    }

    return host;
}

/**
 *判断是字符串str是不是以start开始
 */
int is_start_with(const char *str, char *start)
{
	if (NULL == str || NULL == start)
	{
		return -1;
	}
	int str_len = strlen(str);
	int start_len = strlen(start);
	if (str_len < start_len || str_len == 0 || start_len == 0)
	{
		return -1;
	}
	char *p = start;
	int i = 0;
	while(*p != '\0')
	{
		if (str[i] != *p)
		{
			return -1;
		}
		++p;
		++i;
	}
	return 0;
}
 
/**
 *判断是字符串str是不是以end结束
 */
int is_end_with(const char *str, char *end)
{
	if (NULL == str || NULL == end)
	{
		return -1;
	}
	int str_len = strlen(str);
	int end_len = strlen(end);
	if (str_len < end_len || str_len == 0 || end_len == 0)
	{
		return -1;
	}	
	while (end_len > 0)
	{
		if (str[str_len - 1] != end[end_len - 1])
		{
			return -1;
		}
		--str_len;
		--end_len;
	}
	return 0;
}



