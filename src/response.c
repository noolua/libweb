//
//  response.c
//  miniweb
//
//  Created by rockee on 14-1-22.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "response.h"

#define WEB_DEFAULT_RESPONSE \
"HTTP/1.1 200 OK\r\n" \
"Content-Type: text/plain\r\n" \
"Content-Length: 12\r\n" \
"\r\n" \
"Hello World\n" \

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

mweb_http_response_t *mweb_http_response_create(uv_tcp_t* stream, mweb_http_response_send_complete_cb response_send_complete_cb, void* connection){
    mweb_http_response_t *response = malloc(sizeof(mweb_http_response_t));
    response->req.data = response;
    response->response_send_complete_cb = response_send_complete_cb;
    response->connection = connection;
    response->buf = uv_buf_init(WEB_DEFAULT_RESPONSE, strlen(WEB_DEFAULT_RESPONSE));
    if(uv_write(&response->req, (uv_stream_t*)stream, &response->buf, 1, web_response_after_write_cb)){
        ERR("Send response failed\n");
    }
    return response;
}

void mweb_http_response_destory(mweb_http_response_t* response){
    free(response);
}
