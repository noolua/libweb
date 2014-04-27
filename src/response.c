//
//  response.c
//  miniweb
//
//  Created by rockee on 14-1-22.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static mweb_response_lua_context_t *mweb_lua_context_create(mweb_http_connection_t *cnn, const char* filepath){
    const char *res = "{\"error\":-1}";
    mweb_response_lua_context_t *context = mweb_alloc(sizeof(mweb_response_lua_context_t));
    if(context){
        lua_State *L = NULL;
        int error = -1;
        if(cnn->server->L){
            L = lua_newthread(cnn->server->L);
            context->is_lua_thread = 1;
        }else{
            L = luaL_newstate();
            luaL_openlibs(L);
            context->is_lua_thread = 0;
        }
        context->L = L;
        if(filepath){
            error = luaL_loadfile(L, filepath);            
            lua_pushstring(L, cnn->request->url.base);
            lua_pcall(L, 1, 1, 0);
            if(error){
                ERR("%s", lua_tostring(L, -1));
                lua_pop(L, 1);
                MWSTRING_COPY_CSTRING(context->res, res, strlen(res));
            }else{
                res = luaL_checkstring(L, -1);
                MWSTRING_COPY_CSTRING(context->res, res, strlen(res));
            }
        }
    }
    return context;
}

static void mweb_file_context_destory(mweb_response_file_context_t* context){
    fclose(context->fp);
    mweb_free(context);
}

static void mweb_text_context_destory(mweb_response_text_context_t* context){
    MWSTRING_RELEASE(context->text);
    mweb_free(context);
}

static void mweb_lua_context_destory(mweb_response_lua_context_t* context){
    MWSTRING_RELEASE(context->res);
    if(!context->is_lua_thread){
        lua_close(context->L);
    }
    mweb_free(context);
}

static void mweb_response_after_write_cb(uv_write_t* req, int status) {
    mweb_http_response_t *response = (mweb_http_response_t*)req->data;
    if(response->type == response_type_404)
        response->response_send_complete_cb(response->connection, status);
    else if(response->type == response_type_file){
        mweb_response_file_context_t *context = (mweb_response_file_context_t *)response->context;
        if(mweb_file_context_read(context) > 0){
            response->buf = uv_buf_init(context->chunk, (unsigned int)context->chunk_len);
            if(uv_write(&response->req, (uv_stream_t*)response->stream, &response->buf, 1, mweb_response_after_write_cb)){
                ERR("Send response failed\n");
            }
        }else{
            mweb_file_context_destory(context);
            response->context = NULL;
            response->response_send_complete_cb(response->connection, status);
        }
    }else if(response->type == response_type_text){
        mweb_response_text_context_t *context = (mweb_response_text_context_t *)response->context;
        mweb_text_context_destory(context);
        response->context = NULL;
        response->response_send_complete_cb(response->connection, status);        
    }else if(response->type == response_type_lua){
        mweb_response_lua_context_t *context = (mweb_response_lua_context_t *)response->context;
        mweb_lua_context_destory(context);
        response->context = NULL;
        response->response_send_complete_cb(response->connection, status);        
    }
}

static mweb_http_response_t *mweb_http_response_404(mweb_http_connection_t *cnn, mweb_http_response_send_complete_cb response_send_complete_cb){
    mweb_http_response_t *response = mweb_alloc(sizeof(mweb_http_response_t));
    response->type = response_type_404;
    response->req.data = response;
    response->response_send_complete_cb = response_send_complete_cb;
    response->connection = cnn;
    response->context = NULL;
    response->buf = uv_buf_init((char*)response_404, sizeof(response_404));
    if(uv_write(&response->req, (uv_stream_t*)cnn->stream, &response->buf, 1, mweb_response_after_write_cb)){
        ERR("Send response failed\n");
    }
    return response;
}

static mweb_http_response_t *mweb_http_response_file(mweb_http_connection_t *cnn, mweb_http_response_send_complete_cb response_send_complete_cb, const char* filepath){
    mweb_http_response_t *response = NULL;
    mweb_response_file_context_t *context = mweb_file_context_create(filepath);
    if(context){
        response = mweb_alloc(sizeof(mweb_http_response_t));
        response->type = response_type_file;
        response->req.data = response;
        response->response_send_complete_cb = response_send_complete_cb;
        response->connection = cnn;
        response->context = context;
        response->stream = cnn->stream;
        if(mweb_file_context_read(context) > 0){
            response->buf = uv_buf_init(context->chunk, (unsigned int)context->chunk_len);
            if(uv_write(&response->req, (uv_stream_t*)response->stream, &response->buf, 1, mweb_response_after_write_cb)){
                ERR("Send response failed\n");
            }
        }
    }else{
        return mweb_http_response_404(cnn, response_send_complete_cb);
    }
    return response;
}

/*
static mweb_http_response_t *mweb_http_response_text(mweb_http_connection_t *cnn, mweb_http_response_send_complete_cb response_send_complete_cb, const char* text){
    mweb_http_response_t *response = mweb_alloc(sizeof(mweb_http_response_t));
    mweb_response_text_context_t *context = mweb_alloc(sizeof(mweb_response_text_context_t));
    if(context){
        MWSTRING_COPY_CSTRING(context->text, text, strlen(text));
        response->type = response_type_text;
        response->req.data = response;
        response->response_send_complete_cb = response_send_complete_cb;
        response->connection = cnn;
        response->context = context;
        response->buf = uv_buf_init((char*)context->text.base, context->text.len);
        if(uv_write(&response->req, (uv_stream_t*)cnn->stream, &response->buf, 1, mweb_response_after_write_cb)){
            ERR("Send response failed\n");
        }
    }else{
        return mweb_http_response_404(cnn, response_send_complete_cb);
    }
    return response;    
}
*/
static mweb_http_response_t *mweb_http_response_lua(mweb_http_connection_t *cnn, mweb_http_response_send_complete_cb response_send_complete_cb, const char* filepath){
    mweb_http_response_t *response = NULL;
    mweb_response_lua_context_t *context = mweb_lua_context_create(cnn, filepath);
    if(context){
        response = mweb_alloc(sizeof(mweb_http_response_t));
        response->type = response_type_lua;
        response->req.data = response;
        response->response_send_complete_cb = response_send_complete_cb;
        response->connection = cnn;
        response->context = context;
        response->stream = cnn->stream;
        response->buf = uv_buf_init((char*)context->res.base, context->res.len);
        if(uv_write(&response->req, (uv_stream_t*)response->stream, &response->buf, 1, mweb_response_after_write_cb)){
            ERR("Send response failed\n");
        }
    }else{
        return mweb_http_response_404(cnn, response_send_complete_cb);
    }
    return response;   
}

int mweb_http_response(mweb_http_connection_t *cnn, mweb_http_response_send_complete_cb response_send_complete_cb){
    char filepath[1024];
    const char *root = cnn->server->wwwroot;
    const char *url = cnn->request->url.base;
    if(strcmp(url, "/") == 0){
        url = "/index.html";
    }
    const char *tk = strstr(url, "?");
    if(tk){
        char dp[1024];
        strncpy(dp, url, 1023);
        dp[tk-url] = 0;
        sprintf(filepath, "%s%s", root, dp);
    }else{
        sprintf(filepath, "%s%s", root, url);
    }
    if(filepath[strlen(filepath) - 1] == '/'){
        cnn->response = mweb_http_response_404(cnn, response_send_complete_cb);
    }else if(strcmp(filepath + strlen(filepath) - 4, ".lua") == 0){
        cnn->response = mweb_http_response_lua(cnn, response_send_complete_cb, filepath);
    }else{
        cnn->response = mweb_http_response_file(cnn, response_send_complete_cb, filepath);
    }

    return 0;
}

void mweb_http_response_destory(mweb_http_response_t* response){
    mweb_free(response);
}
