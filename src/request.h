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

#define MAX_MWEB_HTTP_HEADERS 20

typedef struct mweb_http_header_s{
    const char* field;
    const char* value;
    size_t field_length;
    size_t value_length;
}mweb_http_header_t;

typedef struct mweb_http_request_s{
    uv_write_t req;
    uv_stream_t stream;
    uv_stream_t *server;
    http_parser parser;
    char* url;
    char* method;
    int header_lines;
    mweb_http_header_t headers[MAX_MWEB_HTTP_HEADERS];
    const char* body;
}mweb_http_request_t;

http_parser_settings *mweb_global_http_parser_settings();

#endif
