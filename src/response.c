//
//  response.c
//  miniweb
//
//  Created by rockee on 14-1-22.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//

#include "response.h"

#define CRLF "\r\n"
static const char response_404[] =
"HTTP/1.1 404 Not Found" CRLF
"Server: libmweb/master" CRLF
"Date: Thu, 23 Jan 2014 06:39:53 UTC" CRLF
"Connection: Keep-Alive" CRLF
"Content-Type: text/html" CRLF
"Content-Length: 16" CRLF
CRLF
"404 Not Found" CRLF
;

struct mweb_http_response_s{
    uv_write_t req;
    uv_buf_t buf;
    mweb_http_response_send_complete_cb response_send_complete_cb;
    void* connection;
};

static void web_response_after_write_cb(uv_write_t* req, int status) {
    mweb_http_response_t *response = (mweb_http_response_t*)req->data;
    response->response_send_complete_cb(response->connection, status);
}

mweb_http_response_t *mweb_http_response_create_404(uv_tcp_t* stream, mweb_http_response_send_complete_cb response_send_complete_cb, void* connection){
    mweb_http_response_t *response = mweb_alloc(sizeof(mweb_http_response_t));
    response->req.data = response;
    response->response_send_complete_cb = response_send_complete_cb;
    response->connection = connection;
    response->buf = uv_buf_init((char*)response_404, sizeof(response_404));
    if(uv_write(&response->req, (uv_stream_t*)stream, &response->buf, 1, web_response_after_write_cb)){
        ERR("Send response failed\n");
    }
    return response;
}

void mweb_http_response_destory(mweb_http_response_t* response){
    mweb_free(response);
}
