//
//  request.h
//  miniweb
//
//  Created by rockee on 14-1-22.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//

#ifndef miniweb_request_h
#define miniweb_request_h

#include "mweb.h"

typedef void (*mweb_http_request_parser_complete_cb)(void* connection, int status);
typedef struct mweb_http_request_s mweb_http_request_t;

mweb_http_request_t *mweb_http_request_create(mweb_http_request_parser_complete_cb parser_complete_cb, void* connection);
void mweb_http_request_destory(mweb_http_request_t* request);
size_t mweb_http_request_parser(mweb_http_request_t* request, const char* base, size_t len);

#endif
