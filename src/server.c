//
//  server.c
//  miniweb
//
//  Created by rockee on 14-1-22.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "connection.h"
#include "server.h"

int MWEB_QUIET = 0;
int MWEB_SYSLOG = 0;

typedef struct mweb_server_s{
    uv_tcp_t server;
    uv_loop_t *loop;
    const char* wwwroot;
    int cacheoff;
}mweb_server_t;

static void web_connection_close_cb(uv_handle_t* handle){
    mweb_http_connection_t *connection = (mweb_http_connection_t*)handle->data;
    mweb_http_connection_destory(connection);
    mweb_free(handle);
}

static void web_connection_after_shutdown_cb(uv_shutdown_t* req, int status){
    uv_close((uv_handle_t*)req->handle, web_connection_close_cb);
    mweb_free(req);
}

static void web_connection_should_shutdown_cb(uv_stream_t* stream, int status){
    uv_shutdown_t *req;
    req = mweb_alloc(sizeof(uv_shutdown_t));
    uv_shutdown(req, (uv_stream_t*)stream, web_connection_after_shutdown_cb);
}

static void web_alloc_buffer_cb(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf){
    buf->base = mweb_alloc(suggested_size);
    buf->len = suggested_size;
}

static void web_request_after_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    if (nread < 0) {
        if(nread != UV_EOF)
            ERR("Error on reading: %s.\n", uv_strerror((int)nread));
        web_connection_should_shutdown_cb(stream, 0);
    }
    if(nread > 0){
        mweb_http_connection_t *connection = (mweb_http_connection_t *)stream->data;
        
        size_t parsed = mweb_http_connection_parser(connection, buf->base, nread);
        if (parsed < nread){
            ERR("Error on parsing HTTP request: \n");
            web_connection_should_shutdown_cb(stream, 0);
        }
    }
    mweb_free(buf->base);
}

static void web_new_connection_cb(uv_stream_t* server, int status) {
    uv_tcp_t* accept_stream;
    mweb_http_connection_t *connection;
    mweb_server_t *web = (mweb_server_t*)server->data;
    if (status < 0) {
        ERR("Error on connection: %s.\n", uv_strerror(status));
        return;
    }
    accept_stream = mweb_alloc(sizeof(uv_tcp_t));
    uv_tcp_init(web->loop, (uv_tcp_t*) accept_stream);
    connection = mweb_http_connection_create(accept_stream, web_connection_should_shutdown_cb);
    accept_stream->data = connection;
    if (!connection) {
        ERR("create http connection failed\n");
        return;
    }
    
    if (uv_accept(server, (uv_stream_t*)accept_stream) == 0) {
        uv_read_start((uv_stream_t*)accept_stream, web_alloc_buffer_cb, web_request_after_read_cb);
    } else {
        mweb_http_connection_destory(connection);
    }
}

static mweb_server_t *web = NULL;
static void web_server_close_cb(uv_handle_t* server){
    mweb_server_t *close_web = (mweb_server_t*)server->data;
    mweb_free(close_web);
    web = NULL;
}

/******************************************************************
 * public interfaces
 ******************************************************************/

int mweb_startup(uv_loop_t *loop, const char *address, int port, const char* wwwroot, int cacheoff){
    int ret = -1;
    if (!web) {
        struct sockaddr_in listen_address;
        web = mweb_alloc(sizeof(mweb_server_t));
        web->loop = loop;
        web->wwwroot = wwwroot;
        web->cacheoff = cacheoff;
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

const char* mweb_root(){
    if(web){
        return web->wwwroot;
    }
    return NULL;
}
