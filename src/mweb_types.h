//
//  mweb_types.h
//  miniweb
//
//  Created by rockee on 14-4-27.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//
#ifndef mweb_types_h
#define mweb_types_h
#include "http_parser.h"

#define FILE_CHUNK_SIZE (65536)

typedef void (*mweb_http_connection_should_close_cb)(uv_stream_t*stream, int status);
typedef void (*mweb_http_response_send_complete_cb)(void* connection, int status);
typedef void (*mweb_http_request_parser_complete_cb)(void* connection, int status);

enum ResponseType {
    response_type_404 = 0,
    response_type_file = 1,
    response_type_lua = 2,
};

typedef struct mweb_http_response_s{
    int type;
    uv_write_t req;
    uv_buf_t buf;
    uv_tcp_t *stream;
    mweb_http_response_send_complete_cb response_send_complete_cb;
    void* connection;
    void* context;
}mweb_http_response_t;

typedef struct mweb_response_file_context_s{
    FILE *fp;
    char chunk[FILE_CHUNK_SIZE];
    size_t chunk_len;
    size_t file_len;
    size_t read_bytes;
}mweb_response_file_context_t;

typedef struct mweb_http_header_s{
    mweb_string_t field;
    mweb_string_t value;
    struct mweb_http_header_s *next;
}mweb_http_header_t;

typedef struct mweb_http_request_s{
    void *connection;
    mweb_http_request_parser_complete_cb parser_complete_cb;
    http_parser parser;
    mweb_string_t url;
    mweb_string_t method;
    mweb_string_t body;
    mweb_http_header_t *header_first;
}mweb_http_request_t;

typedef struct mweb_server_s{
    uv_tcp_t server;
    uv_loop_t *loop;
    const char* wwwroot;
    int cacheoff;
    mweb_request_handle_cb req_hd_cb;
    void* req_hd_cb_data;
}mweb_server_t;

typedef struct mweb_http_connection_s{
    uv_tcp_t *stream;
    mweb_server_t *server;
    mweb_http_request_t *request;
    mweb_http_response_t *response;
    mweb_http_connection_should_close_cb connection_should_close_cb;
}mweb_http_connection_t;


#endif // mweb_types_h