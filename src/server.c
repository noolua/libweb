//
//  server.c
//  miniweb
//
//  Created by rockee on 14-1-22.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "request.h"
#include "server.h"

int MWEB_QUIET = 0;
int MWEB_SYSLOG = 0;

typedef struct mweb_server_s{
    http_parser_settings *settings;
    uv_tcp_t server;
    uv_loop_t *loop;
}mweb_server_t;

static void web_alloc_buffer_cb(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf){
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

static void web_request_after_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    mweb_http_request_t* http_request = stream->data;
    mweb_server_t *web = (mweb_server_t*)http_request->server->data;
    
    if (nread < 0) {
        ERR("Error on reading: %s.\n", uv_strerror((int)nread));
        uv_close((uv_handle_t*) stream, NULL);
    }
    
    size_t parsed = http_parser_execute(&http_request->parser, web->settings, buf->base, nread);
    
    if (parsed < nread){
        ERR("Error on parsing HTTP request: \n");
        uv_close((uv_handle_t*) stream, NULL);
    }
    free(buf->base);
}

static void web_connection_close_cb(uv_handle_t* handle){
    mweb_http_request_t* http_request = (mweb_http_request_t*)handle->data;
    free(http_request);
}

static void web_new_connection_cb(uv_stream_t* server, int status) {
    mweb_server_t *web = (mweb_server_t*)server->data;
    mweb_http_request_t* http_request = malloc(sizeof(mweb_http_request_t));
    
    if (status < 0) {
        ERR("Error on connection: %s.\n", uv_strerror(status));
        return;
    }
    
    uv_tcp_init(web->loop, (uv_tcp_t*) &http_request->stream);
    http_request->stream.data = http_request;
    http_request->parser.data = http_request;
    http_request->server = server;
    
    if (uv_accept(server, &http_request->stream) == 0) {
        http_parser_init(&http_request->parser, HTTP_REQUEST);
        uv_read_start(&http_request->stream, web_alloc_buffer_cb, web_request_after_read_cb);
    } else {
        uv_close((uv_handle_t*) &http_request->stream, web_connection_close_cb);
    }
}

static mweb_server_t *web = NULL;
static void web_server_close_cb(uv_handle_t* server){
    mweb_server_t *close_web = (mweb_server_t*)server->data;
    free(close_web);
    web = NULL;
}

/******************************************************************
 * public interfaces
 ******************************************************************/

int mweb_startup(uv_loop_t *loop, const char *address, int port){
    int ret = -1;
    if (!web) {
        struct sockaddr_in listen_address;
        web = malloc(sizeof(mweb_server_t));
        web->loop = loop;
        web->settings = mweb_global_http_parser_settings();
        uv_ip4_addr(address, port, &listen_address);
        uv_tcp_init(web->loop, &web->server);
        uv_tcp_bind(&web->server, (const struct sockaddr*)&listen_address);
        web->server.data = web;
        
        if ((ret = uv_listen((uv_stream_t*) &web->server, SOMAXCONN, web_new_connection_cb)) < 0){
            ERR("Error on tcp listen: %s.\n", uv_strerror(ret));
        }
    }else{
        ERR("http server is already startup\n");
    }
    return ret;
}

int mweb_is_running(){
    return web ? 1:0;
}

int mweb_cleanup(){
    if (web) {
        uv_close((uv_handle_t*)&web->server, web_server_close_cb);
    }
    return 0;
}

void mweb_quiet_syslog(int quiet, int syslog){
    MWEB_QUIET = quiet;
    MWEB_SYSLOG = syslog;
}
