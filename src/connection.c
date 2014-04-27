//
//  connection.c
//  miniweb
//
//  Created by rockee on 14-1-23.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "request.h"
#include "response.h"
#include "connection.h"


static void on_http_response_send_complete_cb(void* connection, int status){
    mweb_http_connection_t *cnn = (mweb_http_connection_t*)connection;
    cnn->connection_should_close_cb((uv_stream_t*)cnn->stream, status);
}

static void on_http_parser_complete_cb(void* connection, int status){
    mweb_http_connection_t *cnn = (mweb_http_connection_t*)connection;
    mweb_http_response(cnn, on_http_response_send_complete_cb);
}

mweb_http_connection_t *mweb_http_connection_create(mweb_server_t* server, uv_tcp_t *stream,
                                                    mweb_http_connection_should_close_cb connection_should_close_cb){
    mweb_http_connection_t *connection = mweb_alloc(sizeof(mweb_http_connection_t));
    
    connection->server = server;
    connection->stream = stream;
    connection->request = mweb_http_request_create(on_http_parser_complete_cb, connection);
    connection->response = NULL;
    connection->connection_should_close_cb = connection_should_close_cb;
    return connection;
}

size_t mweb_http_connection_parser(mweb_http_connection_t* connection, const char* data, size_t len){
    return mweb_http_request_parser(connection->request, data, len);
}

void mweb_http_connection_destory(mweb_http_connection_t* connection){
    if (connection->request)
        mweb_http_request_destory(connection->request);
    if(connection->response)
        mweb_http_response_destory(connection->response);
    mweb_free(connection);
}
