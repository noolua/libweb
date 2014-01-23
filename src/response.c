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

#define FILE_CHUNK_SIZE (65536)

enum ResponseType {
    response_type_404 = 0,
    response_type_file = 1,
    response_type_lua = 2,
};

struct mweb_http_response_s{
    int type;
    uv_write_t req;
    uv_buf_t buf;
    mweb_http_response_send_complete_cb response_send_complete_cb;
    void* connection;
    void* context;
};

typedef struct mweb_response_file_context_s{
    FILE *fp;
    char chunk[FILE_CHUNK_SIZE];
    size_t chunk_len;
    size_t file_len;
    size_t read_bytes;
}mweb_response_file_context_t;

static mweb_response_file_context_t *mweb_file_context_create(const char* filepath){
    mweb_response_file_context_t * context = mweb_alloc(sizeof(mweb_response_file_context_t));
    context->read_bytes = 0;
    context->fp = NULL;
    context->chunk_len = 0;
    FILE *fp = fopen(filepath, "r");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        context->file_len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        context->fp = fp;
    }else{
        mweb_free(context);
        context = NULL;
    }
    return context;
}

static size_t mweb_file_context_read(mweb_response_file_context_t *context){
    size_t max_read = (context->file_len - context->read_bytes) > FILE_CHUNK_SIZE ? FILE_CHUNK_SIZE : (context->file_len - context->read_bytes);
    if(max_read > 0){
        context->chunk_len = fread(context->chunk, 1, max_read, context->fp);
        context->read_bytes += context->chunk_len;
    }else{
        context->chunk_len = 0;
    }
    return context->chunk_len;
}

static void mweb_file_context_destory(mweb_response_file_context_t* context){
    fclose(context->fp);
    mweb_free(context);
}

static void mweb_response_after_write_cb(uv_write_t* req, int status) {
    mweb_http_response_t *response = (mweb_http_response_t*)req->data;
    if(response->type == response_type_404)
        response->response_send_complete_cb(response->connection, status);
    else if(response->type == response_type_file){
        /* fixedme */
        /*send file until finished*/
        mweb_response_file_context_t *context = (mweb_response_file_context_t *)response->context;
        mweb_file_context_destory(context);
        response->context = NULL;
        response->response_send_complete_cb(response->context, status);
    }
}

mweb_http_response_t *mweb_http_response_404(uv_tcp_t* stream, mweb_http_response_send_complete_cb response_send_complete_cb, void* connection){
    mweb_http_response_t *response = mweb_alloc(sizeof(mweb_http_response_t));
    response->type = response_type_404;
    response->req.data = response;
    response->response_send_complete_cb = response_send_complete_cb;
    response->connection = connection;
    response->context = NULL;
    response->buf = uv_buf_init((char*)response_404, sizeof(response_404));
    if(uv_write(&response->req, (uv_stream_t*)stream, &response->buf, 1, mweb_response_after_write_cb)){
        ERR("Send response failed\n");
    }
    return response;
}

mweb_http_response_t *mweb_http_response_file(uv_tcp_t* stream, mweb_http_response_send_complete_cb response_send_complete_cb, void* connection, const char* filepath){
    mweb_http_response_t *response = NULL;
    mweb_response_file_context_t *context = mweb_file_context_create(filepath);
    if(context){
        mweb_http_response_t *response = mweb_alloc(sizeof(mweb_http_response_t));
        response->type = response_type_file;
        response->req.data = response;
        response->response_send_complete_cb = response_send_complete_cb;
        response->connection = connection;
        response->context = context;
        if(mweb_file_context_read(context) > 0){
            response->buf = uv_buf_init(context->chunk, (unsigned int)context->chunk_len);
            if(uv_write(&response->req, (uv_stream_t*)stream, &response->buf, 1, mweb_response_after_write_cb)){
                ERR("Send response failed\n");
            }
        }
    }else{
        return mweb_http_response_404(stream, response_send_complete_cb, connection);
    }
    return response;
}

void mweb_http_response_destory(mweb_http_response_t* response){
    mweb_free(response);
}
