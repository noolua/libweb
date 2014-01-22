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
#include "response.h"

#define WEB_DEFAULT_RESPONSE \
"HTTP/1.1 200 OK\r\n" \
"Content-Type: text/plain\r\n" \
"Content-Length: 12\r\n" \
"\r\n" \
"Hello World\n" \

static void web_response_write_cb(uv_write_t* req, int status) {
    uv_close((uv_handle_t*) req->handle, NULL);
}

static int on_http_message_begin_cb(http_parser* parser) {
    mweb_http_request_t* http_request = parser->data;
    http_request->header_lines = 0;
    return 0;
}

static int on_http_url_cb(http_parser* parser, const char* chunk, size_t len) {
    mweb_http_request_t* http_request = parser->data;
    
    http_request->url = malloc(len+1);
    
    strncpy((char*) http_request->url, chunk, len);
    
    return 0;
}

static int on_http_header_field_cb(http_parser* parser, const char* chunk, size_t len) {
    mweb_http_request_t* http_request = parser->data;
    
    mweb_http_header_t* header = &http_request->headers[http_request->header_lines];
    
    header->field = malloc(len+1);
    header->field_length = len;
    
    strncpy((char*) header->field, chunk, len);
    
    return 0;
}

static int on_http_header_value_cb(http_parser* parser, const char* chunk, size_t len) {
    mweb_http_request_t* http_request = parser->data;
    
    mweb_http_header_t* header = &http_request->headers[http_request->header_lines];
    
    header->value_length = len;
    header->value = malloc(len+1);
    
    strncpy((char*) header->value, chunk, len);
    
    ++http_request->header_lines;
    
    return 0;
}

static int on_http_headers_complete_cb(http_parser* parser) {
    mweb_http_request_t* http_request = parser->data;
    
    const char* method = http_method_str(parser->method);
    
    http_request->method = malloc(sizeof(method));
    strncpy(http_request->method, method, strlen(method));
    
    return 0;
}

static int on_http_body_cb(http_parser* parser, const char* chunk, size_t len) {
    mweb_http_request_t* http_request = parser->data;
    
    http_request->body = malloc(len+1);
    http_request->body = chunk;
    
    return 0;
}

static int on_http_message_complete_cb(http_parser* parser) {
    mweb_http_request_t* http_request = parser->data;
    
    /* now print the ordered http http_request to console */
    LOG("url: %s\n", http_request->url);
    LOG("method: %s\n", http_request->method);
    for (int i = 0; i < 5; i++) {
        mweb_http_header_t* header = &http_request->headers[i];
        if (header->field)
            LOG("Header: %s: %s\n", header->field, header->value);
    }
    LOG("body: %s\n", http_request->body);
    LOG("\r\n");
    
    /* lets send our short http hello world response and close the socket */
    uv_buf_t buf;
    buf.base = WEB_DEFAULT_RESPONSE;
    buf.len = strlen(WEB_DEFAULT_RESPONSE);
    uv_write(&http_request->req, &http_request->stream, &buf, 1, web_response_write_cb);
    return 0;
}

static int on_http_status_cb(http_parser* parser, const char* chunk, size_t len){
    return 0;
}

static http_parser_settings global_settings = {
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
http_parser_settings *mweb_global_http_parser_settings(){
    return &global_settings;
}


