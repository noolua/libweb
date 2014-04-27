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
#include "mweb_types.h"

mweb_http_connection_t *mweb_http_connection_create(mweb_server_t* server, uv_tcp_t *stream,
                                                    mweb_http_connection_should_close_cb connection_should_close_cb);
size_t mweb_http_connection_parser(mweb_http_connection_t* connection, const char* data, size_t len);

void mweb_http_connection_destory(mweb_http_connection_t* connection);

#endif
