//
//  connection.h
//  miniweb
//
//  Created by rockee on 14-1-23.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//

#ifndef miniweb_connection_h
#define miniweb_connection_h

#include "mweb.h"
#include "request.h"
#include "response.h"

typedef void (*mweb_http_connection_should_close_cb)(uv_stream_t*stream, int status);

typedef struct mweb_http_connection_s mweb_http_connection_t;

mweb_http_connection_t *mweb_http_connection_create(uv_tcp_t *stream,
                                                    mweb_http_connection_should_close_cb connection_should_close_cb);
size_t mweb_http_connection_parser(mweb_http_connection_t* connection, const char* data, size_t len);

void mweb_http_connection_destory(mweb_http_connection_t* connection);

#endif
