#include <dirent.h>
#include <dlfcn.h>
#include <libgen.h>
#include <string.h>
#include <dslink/auth.h>
//#define WINDOWS  /* uncomment this line to use it for windows.*/
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
//#include <unistd.h>

#include <wslay_event.h>

#include "broker/handshake.h"
#include "broker/config.h"
#include "broker/data/data.h"
#include "broker/sys/sys.h"

#define LOG_TAG "broker"
#include <dslink/log.h>
#include <dslink/utils.h>

#include <dslink/storage/storage.h>

#include <broker/upstream/upstream_node.h>
#include <broker/utils.h>
#include <broker/net/ws.h>
#include <dslink/socket_private.h>

#define CONN_RESP "HTTP/1.1 200 OK\r\n" \
                    "Connection: close\r\n" \
                    "Access-Control-Allow-Origin: *\r\n" \
                    "Content-Type:application/json; charset=utf-8\r\n" \
                    "Content-Length: %d\r\n" \
                    "\r\n%s\r\n"


#ifndef IOT_DSA_C_SDK_GIT_COMMIT_HASH
#define IOT_DSA_C_SDK_GIT_COMMIT_HASH "unknown"
#endif

uv_loop_t *mainLoop = NULL;

struct Extension
{
    uv_lib_t* handle;
    struct ExtensionCallbacks callbacks;
};

static void handle_post(Broker *broker, HttpRequest *req, Socket *sock) {
    

}
static void handle_admin(Broker *broker, HttpRequest *req, Socket *sock) {

    char current_dir[FILENAME_MAX];
    GetCurrentDir(current_dir, FILENAME_MAX);
    printf("Current working dir: %s\n", current_dir);

    // process configuration
    log_info("admin configuration...\n");
    char path[512];
    if(is_start_with(req->uri.resource, "/static") == 0){
         strcpy(path, "/admin");   
         strcat(path,req->uri.resource);

    }else{
         strcpy(path, req->uri.resource);   
    }
        if (path[strlen(path) - 1] == '/'){
            strcat(path,"index.html");
        }
        if (path[strlen(path) - 1] == '/'){
            strcat(path,"index.html");
        }

    FILE  *ptrFile = fopen(strcat(current_dir,path), "r");
            log_info("admin usr path %s \n",current_dir);
    if(ptrFile==NULL){
        broker_send_not_found_error(sock);
        return;
        }

    long lSize;
   // char *buffer;

    // fprintf( ptrFile, "<HTML>\n");
    // fprintf( ptrFile, "<HEAD> <TITLE> The Geek Stuff </TITLE> </HEAD>\n" ); 
    // fprintf( ptrFile, "<BODY BGCOLOR=\"#110022\" TEXT=\"#FFBBAA\"> \n"); 
    // fprintf( ptrFile, "<p>Example 1: This file was created from edge broker service</p>\n");

    // fprintf( ptrFile, "</BODY>\n"); 
    // fprintf( ptrFile, "</HTML>"); 

    fseek(ptrFile, 0L, SEEK_END);
    lSize = ftell(ptrFile);

    rewind(ptrFile);
    //fclose(ptrFile);

    // buffer = dslink_calloc(1, (int)(lSize + 1));
    // buffer[lSize+1] = '\0';
    // int count = fread(buffer, sizeof(char), (int)lSize, ptrFile);
    // log_info("admin usr path %s size: %d\n",current_dir,count);

    // if (count < 0) {
    //     broker_send_internal_error(sock);
    //     goto exit;
    // }
    
    char buf[1024];

    char* HTML_RESP="HTTP/1.1 200 OK\r\n" \
                    "Connection: Keep-Alive\r\n" \
                    "Access-Control-Allow-Origin: *\r\n" \
                    "Content-Type:%s; charset=utf-8\r\n" \
                    "Content-Length: %d\r\n\r\n";
    const char* mime;
    mime=mime_get_type(path);
    printf("response : %s\n",mime);

    int len = snprintf(buf, sizeof(buf) - 1,
                       HTML_RESP,mime_get_type(path), lSize);
    log_info("read resp length : %d  \n",(int)lSize);

    buf[len] = '\0';
    dslink_socket_write(sock, buf, (size_t) len);  
    char tbuf[1024];
    //fgets(tbuf, sizeof(tbuf), ptrFile);
    int i=0;
    int bytesRead=0;

     while ((bytesRead = fread(tbuf, 1, sizeof(tbuf), ptrFile)) > 0)
  {
              dslink_socket_write(sock, tbuf, bytesRead);  
                  //    log_info("temp read : %d\n",bytesRead);

              i+=bytesRead;
    // process bytesRead worth of data in buffer
  }

    // while (!feof(ptrFile))
    // {
    //     fgets(tbuf, sizeof(tbuf), ptrFile);
    //     dslink_socket_write(sock, tbuf, strlen(tbuf));  
    //     i+=strlen(tbuf);
    //    // send(client, tbuf, strlen(tbuf), 0);

    // }
        log_info("total read : %d\n",i);

  //  dslink_socket_write(sock, buffer, (size_t) (lSize+1));  
   // dslink_free(buffer);
    return;

exit:
    if (ptrFile) {
        fclose( ptrFile );
    }
    return;

}
static
void handle_about(Broker *broker, HttpRequest *req, Socket *sock) {
     log_info("handle about %s \n",req->uri.resource);
     json_t *resp= json_object();
     json_object_set_new_nocheck(resp, "version", json_string_nocheck("20200530"));
     json_object_set_new_nocheck(resp, "copyRight", json_string_nocheck("GinLink"));
     json_object_set_new_nocheck(resp, "pods", json_string_nocheck("modbus,bacnet"));
     json_object_set_new_nocheck(resp, "license", json_string_nocheck("XXXXXXXXXX"));
        char *data = json_dumps(resp, JSON_INDENT(2));
        json_decref(resp);
      char buf[1024];
    int len = snprintf(buf, sizeof(buf) - 1,
                       CONN_RESP, (int) strlen(data), data);
    buf[len] = '\0';
    dslink_free(data);
    dslink_socket_write(sock, buf, (size_t) len);  

}
static
void handle_login(Broker *broker, HttpRequest *req, Socket *sock) {
     //log_info("handle about %s \n",req->uri.resource);
     log_info("Elapsed time:%u secs./n",clock()/CLOCKS_PER_SEC);

     json_t *resp= json_object();
     AccessToken ac={"","","",3};
     json_object_set_new_nocheck(resp, "access_token", json_string_nocheck("20200530"));
     json_object_set_new_nocheck(resp, "token_type", json_string_nocheck("bearer"));
     json_object_set_new_nocheck(resp, "refresh_token", json_string_nocheck("modbus,bacnet"));
     json_object_set_new_nocheck(resp, "expires_in", json_integer(86399));
     json_object_set_new_nocheck(resp, "scope", json_string_nocheck("all"));

        char *data = json_dumps(resp, JSON_INDENT(2));
        json_decref(resp);
      char buf[1024];
    int len = snprintf(buf, sizeof(buf) - 1,
                       CONN_RESP, (int) strlen(data), data);
    buf[len] = '\0';
    dslink_free(data);
    dslink_socket_write(sock, buf, (size_t) len);  

}


static
void handle_api(Broker *broker, HttpRequest *req, Socket *sock) {
 json_error_t err;
    json_t *body;
    {
        const char *start = strchr(req->body, '{');
        const char *end = strrchr(req->body, '}');
        if (!(start && end)) {
            return;
        }
        body = json_loadb(start, end - start + 1, 0, &err);
        if (!body) {
            broker_send_internal_error(sock);
            return;
        }
    }

     log_info("get payload %s abc\n",json_string_value(json_object_get(body, "action")));
     json_t *resp= json_object();
     json_object_set_new_nocheck(resp, "version", json_string_nocheck("20200530"));
     json_object_set_new_nocheck(resp, "copyRight", json_string_nocheck("GinLink"));
     json_object_set_new_nocheck(resp, "pods", json_string_nocheck("modbus,bacnet"));
     json_object_set_new_nocheck(resp, "license", json_string_nocheck("XXXXXXXXXX"));
        char *data = json_dumps(resp, JSON_INDENT(2));
        json_decref(resp);
      char buf[1024];
    int len = snprintf(buf, sizeof(buf) - 1,
                       CONN_RESP, (int) strlen(data), data);
    buf[len] = '\0';
    dslink_free(data);
    log_info("resp:%s \n", buf);
    dslink_socket_write(sock, buf, (size_t) len);  

}

static
void handle_conn(Broker *broker, HttpRequest *req, Socket *sock) {
    json_error_t err;
    char *dsId = NULL;
    json_t *body;
    {
        const char *start = strchr(req->body, '{');
        const char *end = strrchr(req->body, '}');
        if (!(start && end)) {
            goto exit;
        }
        body = json_loadb(start, end - start + 1, 0, &err);
        if (!body) {
            broker_send_internal_error(sock);
            goto exit;
        }
    }

    dsId = dslink_str_unescape(broker_http_param_get(&req->uri, "dsId"));
    if (!dsId) {
        goto exit;
    }
    log_info("%s connecting\n", dsId);
    const char *token = broker_http_param_get(&req->uri, "token");
    json_t *resp = broker_handshake_handle_conn(broker, dsId, token, body);
    json_decref(body);
    if (!resp) {
        broker_send_internal_error(sock);
        goto exit;
    }

    char *data = json_dumps(resp, JSON_INDENT(2));
    json_decref(resp);
    if (!data) {
        broker_send_internal_error(sock);
        goto exit;
    }

    char buf[1024];
    int len = snprintf(buf, sizeof(buf) - 1,
                       CONN_RESP, (int) strlen(data), data);
    buf[len] = '\0';
    dslink_free(data);
    dslink_socket_write(sock, buf, (size_t) len);

exit:
    if (dsId) {
        dslink_free(dsId);
    }
    return;
}

static
int handle_ws(Broker *broker, HttpRequest *req, Client *client) {

    size_t len = 0;
    char *dsId = NULL;
    const char *key = broker_http_header_get(req->headers,
                                             "Sec-WebSocket-Key", &len);
    if (!key) {
        goto fail;
    }
    char accept[64];
    if (broker_ws_generate_accept_key(key, len, accept, sizeof(accept)) != 0) {
        goto fail;
    }

    dsId = dslink_str_unescape(broker_http_param_get(&req->uri, "dsId"));
    const char *auth = broker_http_param_get(&req->uri, "auth");
    if (!(dsId && auth)) {
        goto fail;
    }

    if (broker_handshake_handle_ws(broker, client, dsId,
                                   auth, accept) != 0) {
        goto fail;
    }

    return 0;
fail:
    if (dsId) {
        dslink_free(dsId);
    }
    broker_send_bad_request(client->sock);
    dslink_socket_close_nofree(client->sock);
    return 1;
}

void broker_https_on_data_callback(Client *client, void *data) {
    Broker *broker = data;
    RemoteDSLink *link = client->sock_data;
    if (link) {
        link->ws->read_enabled = 1;
        wslay_event_recv(link->ws);
        if (link->pendingClose) {
            // clear the poll now, so it won't get cleared twice
            link->client->poll = NULL;
            broker_close_link(link);
        }
        return;
    }

    if (client->sock->socket_ctx.fd == -1) {
        goto exit;
    }

    HttpRequest req;
    char buf[1024];
    char bodyBuf[1024];
    {
        int read = dslink_socket_read(client->sock, buf, sizeof(buf) - 1);
        if(read < 0) {
            goto exit;
        }

        buf[read] = '\0';
        int err = broker_http_parse_req(&req, buf);
        if (err) {
            goto exit;
        }
        //only java dslinks sends the body as a separate SSL record
        read = dslink_socket_read(client->sock, bodyBuf, sizeof(bodyBuf) - 1);
        if(read > 0) {
            bodyBuf[read] = '\0';
            req.body = bodyBuf;
        }

    }
    if (strcmp(req.uri.resource, "/conn") == 0) {
        if (strcmp(req.method, "POST") != 0) {
            log_info("invalid method on /conn \n");
            broker_send_bad_request(client->sock);
            goto exit;
        }

        handle_conn(broker, &req, client->sock);
    } else if (strcmp(req.uri.resource, "/ws") == 0) {
        if (strcmp(req.method, "GET") != 0) {
            log_info("invalid method on /ws \n");
            broker_send_bad_request(client->sock);
            goto exit;
        }

        handle_ws(broker, &req, client);
        return;
    } else if (strcmp(req.uri.resource, "/admin") == 0) {
//        if (strcmp(req.mehtod, "POST") != 0) {
//            log_info("invalid method on /admin \n");
//            broker_send_bad_request(client->sock);
//            goto exit;
//        }
        handle_admin(broker, &req, client->sock);
        log_info("deal admin path end \n");

    } else {
        broker_send_not_found_error(client->sock);
    }

    exit:
    dslink_socket_close_nofree(client->sock);
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

void broker_on_data_callback(Client *client, void *data) {

    Broker *broker = data;
    RemoteDSLink *link = client->sock_data;
    if (link) {
        link->ws->read_enabled = 1;
        wslay_event_recv(link->ws);
        if (link->pendingClose) {
            // clear the poll now, so it won't get cleared twice
            link->client->poll = NULL;
            broker_close_link(link);
        }
        return;
    }

    if (client->sock->socket_ctx.fd == -1) {
        goto exit;
    }

    HttpRequest req;
    char buf[1024];
    {
        int read = dslink_socket_read(client->sock, buf, sizeof(buf) - 1);
        if(read < 0) {
            goto exit;
        }
        buf[read] = '\0';
        int err = broker_http_parse_req(&req, buf);
        if (err) {
            goto exit;
        }

    }
        //char *reqPath=;
        log_info("handle request %s \n",req.uri.resource);
        if (strcmp(req.uri.resource, "/login") == 0) {
        if (strcmp(req.method, "POST") != 0) {
            log_info("invalid method on /conn \n");
            //TODO impliment login method
            broker_send_bad_request(client->sock);
            goto exit;
        }else{
            handle_login(broker, &req, client->sock);

        }
        }else if(strcmp(req.uri.resource, "/about") == 0){
        handle_about(broker, &req, client->sock);
        goto exit;
        }else if(is_start_with(req.uri.resource, "/api") == 0){
                  if (strcmp(req.method, "POST") == 0) {
                      handle_api(broker, &req, client->sock);
                  } else{
                      broker_send_bad_request(client->sock);
                  } 
        return;
        }
        if (strcmp(req.uri.resource, "/conn") == 0) {
            if (strcmp(req.method, "POST") != 0) {
            log_info("invalid method on /conn \n");
            broker_send_bad_request(client->sock);
            goto exit;
        }

        handle_conn(broker, &req, client->sock);
        } else if (strcmp(req.uri.resource, "/ws") == 0) {
        if (strcmp(req.method, "GET") != 0) {
            log_info("invalid method on /ws \n");
            broker_send_bad_request(client->sock);
            goto exit;
        }

        handle_ws(broker, &req, client);
        return;
    } else if (is_start_with(req.uri.resource, "/admin") == 0||is_start_with(req.uri.resource, "/static") == 0) {
//        if (strcmp(req.mehtod, "POST") != 0) {
//            log_info("invalid method on /admin \n");
//            broker_send_bad_request(client->sock);
//            goto exit;
//        }
       // mbedtls_net_set_nonblock(ctx);

        handle_admin(broker, &req, client->sock);
    } else if(strcmp(req.method, "POST") == 0){
        handle_post(broker, &req, client->sock);


    }else {
        broker_send_not_found_error(client->sock);
    }

exit:
    dslink_socket_close_nofree(client->sock);
}

void broker_close_link(RemoteDSLink *link) {
    if (!link) {
        return;
    }
    if (link->client) {
        if (link->client->poll) {
            uv_close((uv_handle_t *) link->client->poll,
                     broker_free_handle);
        }
        dslink_socket_close_nofree(link->client->sock);
    }
    if (link->isUpstream) {
      log_info("Upstream %s has disconnected\n", link->name );      
    } else if (link->dsId) {
        log_info("DSLink `%s` has disconnected\n", (char *) link->dsId->data);
    } else {
        log_info("DSLink `%s` has disconnected\n", (char *) link->name);
    }

    ref_t *ref;
    if (link->isUpstream) {
        ref = dslink_map_get(link->broker->upstream->children, (void *) link->name);
    } else {
       ref = dslink_map_get(link->broker->downstream->children, (void *) link->name);
    }

    broker_remote_dslink_free(link);
    // it's possible that free link still rely on node->link to close related streams
    // so link need to be freed before disconnected from node
    if (ref) {
        DownstreamNode *node = ref->data;
        node->link = NULL;
        broker_dslink_disconnect(node);
    }

    dslink_free(link);
}

static
void broker_free(Broker *broker) {
    if (broker->storage) {
        dslink_storage_destroy(broker->storage);
    }

    broker_node_free(broker->root);
    dslink_map_free(&broker->client_connecting);
    dslink_map_free(&broker->remote_pending_sub);
    dslink_map_free(&broker->local_pending_sub);
    memset(broker, 0, sizeof(Broker));
}

static int extension_on_link_connected(Listener *listener, void *node)
{
    Broker* broker = listener->data;
    DownstreamNode *link = node;

    if(list_is_not_empty(&(broker->extensions))) {
        dslink_list_foreach(&(broker->extensions)) {
            struct Extension* extension = ((ListNode*)node)->value;
            if(extension->callbacks.connect_callback) {
                extension->callbacks.connect_callback(link);
            }
        }
    }
    return 0;
}

static int extension_on_link_disconnected(Listener *listener, void *node)
{
    Broker* broker = listener->data;
    DownstreamNode *link = node;

    if(list_is_not_empty(&(broker->extensions))) {
        dslink_list_foreach(&(broker->extensions)) {
            struct Extension* extension = ((ListNode*)node)->value;
            if(extension->callbacks.disconnect_callback) {
                extension->callbacks.disconnect_callback(link);
            }
        }
    }
    return 0;
}

static int extension_on_child_added(Listener *listener, void *node)
{
    DownstreamNode* link = node;
    listener_add(&link->on_link_connected, extension_on_link_connected, listener->data);
    listener_add(&link->on_link_disconnected, extension_on_link_disconnected, listener->data);
    return 0;
}

static
int broker_init(Broker *broker, json_t *defaultPermission) {
    if (dslink_map_init(&broker->client_connecting, dslink_map_str_cmp,
                        dslink_map_str_key_len_cal, dslink_map_hash_key) != 0) {
        goto fail;
    }

    if (dslink_map_init(&broker->remote_pending_sub, dslink_map_str_cmp,
                        dslink_map_str_key_len_cal, dslink_map_hash_key) != 0) {
        goto fail;
    }

    if (dslink_map_init(&broker->local_pending_sub, dslink_map_str_cmp,
                        dslink_map_str_key_len_cal, dslink_map_hash_key) != 0) {
        goto fail;
    }

    broker->root = broker_node_create("", "node");
    if (!broker->root) {
        goto fail;
    }
    broker->root->permissionList = permission_list_load(defaultPermission);

    broker->root->path = dslink_strdup("/");
    json_object_set_new_nocheck(broker->root->meta, "$downstream",
                        json_string_nocheck("/downstream"));

    json_object_set_new_nocheck(broker->root->meta, "$is", json_string_nocheck("dsa/broker"));

    broker->sys = broker_node_create("sys", "static");
    if (!(broker->sys && broker_node_add(broker->root, broker->sys) == 0)) {
        broker_node_free(broker->sys);
        goto fail;
    }

    broker->upstream = broker_node_create("upstream", "static");
    if (!(broker->upstream && broker_node_add(broker->root, broker->upstream) == 0)) {
        broker_node_free(broker->upstream);
        goto fail;
    }

    broker->data = broker_node_create("data", "node");
    if (!(broker->data && broker_node_add(broker->root, broker->data) == 0
          && broker_load_data_nodes(broker) == 0
          && broker_data_node_populate(broker->data) == 0)) {
        broker_node_free(broker->data);
        goto fail;
    }

    broker->downstream = broker_node_create("downstream", "node");
    if (!(broker->downstream
          && broker_node_add(broker->root, broker->downstream) == 0)) {
        broker_node_free(broker->downstream);
        goto fail;
    }

    listener_add(&broker->downstream->on_child_added, extension_on_child_added, broker);

    broker_load_downstream_nodes(broker);
    broker_load_qos_storage(broker);

    if (broker_sys_node_populate(broker->sys)) {
        goto fail;
    }

    BrokerNode *node = broker_node_create("defs", "static");
    if (!(node && json_object_set_new_nocheck(node->meta,
                                              "$hidden",
                                              json_true()) == 0
          && broker_node_add(broker->root, node) == 0)) {
        broker_node_free(node);
        goto fail;
    }


    return 0;
fail:
    broker_free(broker);
    return 1;
}


void broker_stop(Broker* broker) {
    dslink_map_foreach(broker->downstream->children) {
        DownstreamNode *node = entry->value->data;

        // Ensure the dsId is freed
        node->dsId->count = 1;
        dslink_decref(node->dsId);
        node->dsId = NULL;

        if (node->link) {
            RemoteDSLink *link = node->link;
            dslink_socket_close(link->client->sock);
            uv_close((uv_handle_t *) link->client->poll,
                     broker_free_handle);
            dslink_free(link->client);
            link->client = NULL;
            broker_remote_dslink_free(link);
        }
    }
    if(list_is_not_empty(&broker->extensions)) {
        log_info("Deinitializing extensions\n");
        dslink_list_foreach(&broker->extensions) {
            deinit_ds_extension_type deinit_function;

            struct Extension* extension = ((ListNode*)node)->value;

            if(uv_dlsym(extension->handle, "deinit_ds_extension", (void **) &deinit_function) == 0) {
                int ret = deinit_function();
                if(ret == 0) {
                    log_info("Deinitialized extension\n");
                } else {
                    log_err("Could not deinitialize extension: %d\n", ret);
                }
            } else {
                log_warn("No deinitializing function found for extension\n");
            }

            uv_dlclose(extension->handle);
            dslink_free(extension->handle);
            dslink_free(extension);
        }
    }
    dslink_free((void*)broker->extensionConfig.brokerUrl);
    dslink_list_free_all_nodes(&broker->extensions);
}

int broker_init_extensions(Broker* broker, json_t* config) {
    list_init(&broker->extensions);

#ifdef __linux__
    const char* extension_library_name = "libdsmanager.so";
#elif __APPLE__ && __MACH__
    const char* extension_library_name = "libdsmanager.dylib";
#endif

    json_t* extension_library = json_object_get(config, "extension_library");
    if (extension_library && json_is_string(extension_library)) {
        extension_library_name = json_string_value(extension_library);
    }

    struct Extension* extension = dslink_malloc(sizeof(struct Extension));
    extension->handle = (uv_lib_t*)dslink_malloc(sizeof(uv_lib_t));
    extension->callbacks.connect_callback = NULL;
    extension->callbacks.disconnect_callback = NULL;

    if (uv_dlopen(extension_library_name, extension->handle)) {
        log_warn("Could not load extension: '%s': %s\n", extension_library_name, uv_dlerror(extension->handle));
        dslink_free(extension->handle);
        dslink_free(extension);
        return -1;
    } else {
        init_ds_extension_type init_function;
        if(uv_dlsym(extension->handle, "init_ds_extension", (void **)&init_function) != 0) {
            log_debug("Not an extension: '%s' %s\n", extension_library_name, uv_dlerror(extension->handle));
            uv_dlclose(extension->handle);
            dslink_free(extension->handle);
            dslink_free(extension);
            return -1;
        }

        // TODO lfuerste: refactor into a function
        int httpEnabled = 0;
        int httpsEnabled = 0;
        const char *httpHost = NULL;
        char httpPort[8];
        memset(httpPort, 0, sizeof(httpPort));
        {
            json_t *http = json_object_get(config, "http");
            if (http) {
                json_t *enabled = json_object_get(http, "enabled");
                if(enabled && json_boolean_value(enabled)) {
                    httpEnabled = 1;
                    httpHost = json_string_value(json_object_get(http, "host"));

                    json_t *jsonPort = json_object_get(http, "port");
                    if (jsonPort) {
                        json_int_t p = json_integer_value(jsonPort);
                        int len = snprintf(httpPort, sizeof(httpPort) - 1,
                                           "%" JSON_INTEGER_FORMAT, p);
                        httpPort[len] = '\0';
                    }
                }
            }
        }

        const char *httpsHost = NULL;
        char httpsPort[8];
        memset(httpsPort, 0, sizeof(httpsPort));
        {
            json_t *https = json_object_get(config, "https");
            if (https) {
                json_t *enabled = json_object_get(https, "enabled");
                if (enabled && json_boolean_value(enabled)) {
                    httpsEnabled = 1;
                    httpsHost = json_string_value(json_object_get(https, "host"));

                    json_t *jsonPort = json_object_get(https, "port");
                    if (jsonPort) {
                        json_int_t p = json_integer_value(jsonPort);
                        int len = snprintf(httpsPort, sizeof(httpsPort) - 1, "%" JSON_INTEGER_FORMAT, p);
                        httpsPort[len] = '\0';
                    }
                }
            }
        }

        ///

        json_t* extensions_https = json_object_get(config, "extension_https");
        if(extensions_https && json_boolean_value(extensions_https)) {
            if(!httpsEnabled) {
                log_err("Cannot load extensions. At least https has to be enabled.");
                return -1;
            }

            broker->extensionConfig.brokerUrl = setHostFrom("https", httpsHost, httpsPort);
        } else {
            if(!httpEnabled) {
                log_err("Cannot load extensions. At least http has to be enabled.");
                return -1;
            }

            broker->extensionConfig.brokerUrl = setHostFrom("http", httpHost, httpPort);
        }

        broker->extensionConfig.loop = mainLoop;

        if(init_function(broker->sys, &broker->extensionConfig, &extension->callbacks) == 0) {
            log_info("Loaded extension '%s'\n", extension_library_name);
            dslink_list_insert(&(broker->extensions), extension);
        } else {
            log_err("Could not load extension: '%s': initialization failed\n", extension_library_name);
            uv_dlclose(extension->handle);
            dslink_free(extension->handle);
            dslink_free(extension);
            return -1;
        }
    }

    return 0;
}

int broker_start() {
    // onyl allow an uv threadpool of max one thread
    putenv("UV_THREADPOOL_SIZE=1");

    log_info("CHINBOX C-EBS version: %s\n", IOT_DSA_C_SDK_VERSION);
    log_info("CHINBOX C-SDK git commit: %s\n", IOT_DSA_C_SDK_GIT_COMMIT_HASH);

    int ret = 0;
    json_t *config = broker_config_get();
    if (!config) {
        ret = 1;
        return ret;
    }

    Broker broker;
    memset(&broker, 0, sizeof(Broker));

    mainLoop = dslink_calloc(1, sizeof(uv_loop_t));
    uv_loop_init(mainLoop);
    mainLoop->data = &broker;

    json_t *defaultPermission = json_object_get(config, "defaultPermission");

    broker_config_load(config);

    broker.storage = dslink_storage_init(config);
    broker.storage->loop = mainLoop;

    if (broker_init(&broker, defaultPermission) != 0) {
        ret = 1;
        goto exit;
    }

    broker_init_extensions(&broker, config);

    ret = broker_start_server(config);
exit:
    broker_free(&broker);
    dslink_free(mainLoop);
    return ret;
}
