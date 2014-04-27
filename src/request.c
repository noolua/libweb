//
//  server.c
//  miniweb
//
//  Created by rockee on 14-1-22.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//
#include <stdio.h>
#include <string.h>

#include "request.h"

static int on_http_message_begin_cb(http_parser* parser) {
    return 0;
}

static int on_http_url_cb(http_parser* parser, const char* chunk, size_t len) {
    mweb_http_request_t* http_request = parser->data;
    MWSTRING_COPY_CSTRING(http_request->url, chunk, len);
    return 0;
}

static int on_http_header_field_cb(http_parser* parser, const char* chunk, size_t len) {
    mweb_http_request_t* request = parser->data;
    mweb_http_header_t* header = mweb_alloc(sizeof(mweb_http_header_t));
    MWSTRING_INIT(header->field);
    MWSTRING_INIT(header->value);
    header->next = NULL;
    MWSTRING_COPY_CSTRING(header->field, chunk, len);
    header->next = request->header_first;
    request->header_first = header;
    return 0;
}

static int on_http_header_value_cb(http_parser* parser, const char* chunk, size_t len) {
    mweb_http_request_t* request = parser->data;
    mweb_http_header_t* header = request->header_first;
    MWSTRING_COPY_CSTRING(header->value, chunk, len);
    return 0;
}

static int on_http_headers_complete_cb(http_parser* parser) {
    mweb_http_request_t* http_request = parser->data;
    
    const char* method = http_method_str(parser->method);
    MWSTRING_COPY_CSTRING(http_request->method, method, strlen(method));
    return 0;
}

static int on_http_body_cb(http_parser* parser, const char* chunk, size_t len) {
    mweb_http_request_t* http_request = parser->data;
    MWSTRING_COPY_CSTRING(http_request->body, chunk, len);
    return 0;
}

static int on_http_message_complete_cb(http_parser* parser) {
    mweb_http_request_t* http_request = parser->data;
    http_request->parser_complete_cb(http_request->connection, 0);
    return 0;
}

static int on_http_status_cb(http_parser* parser, const char* chunk, size_t len){
    return 0;
}

static http_parser_settings simple_settings = {
    .on_message_begin = on_http_message_begin_cb,
    .on_url = on_http_url_cb,
    .on_status = on_http_status_cb,
    .on_header_field = on_http_header_field_cb,
    .on_header_value = on_http_header_value_cb,
    .on_headers_complete = on_http_headers_complete_cb,
    .on_body = on_http_body_cb,
    .on_message_complete = on_http_message_complete_cb,
};

/******************************************************************
 * public interfaces
 ******************************************************************/

mweb_http_request_t *mweb_http_request_create(mweb_http_request_parser_complete_cb parser_complete_cb, void* connection){
    mweb_http_request_t* http_request = mweb_alloc(sizeof(mweb_http_request_t));
    MWSTRING_INIT(http_request->url);
    MWSTRING_INIT(http_request->method);
    MWSTRING_INIT(http_request->body);
    http_request->header_first = NULL;
    http_request->parser_complete_cb = parser_complete_cb;
    http_request->connection = connection;
    http_parser_init(&http_request->parser, HTTP_REQUEST);
    http_request->parser.data = http_request;
    return http_request;
}

void mweb_http_request_destory(mweb_http_request_t* request){
    mweb_http_header_t *remove = request->header_first;
    MWSTRING_RELEASE(request->url);
    MWSTRING_RELEASE(request->method);
    MWSTRING_RELEASE(request->body);
    while (remove) {
        mweb_http_header_t* next = remove->next;
        MWSTRING_RELEASE(remove->field);
        MWSTRING_RELEASE(remove->value);
        mweb_free(remove);
        remove = next;
    }
    mweb_free(request);
}

size_t mweb_http_request_parser(mweb_http_request_t* request, const char* base, size_t len){
    return http_parser_execute(&request->parser, &simple_settings, base, len);
}
