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
"Connection: Keep-Alive" CRLF
"Content-Type: text/html" CRLF
"Content-Length: 16" CRLF
CRLF
"404 Not Found" CRLF
;

static const char response_lua_begin[] =
"HTTP/1.1 200 Ok" CRLF
"Server: libmweb/master" CRLF
"Connection: Keep-Alive" CRLF
"Content-Type: application/octet-stream" CRLF
"Transfer-Encoding: chunked" CRLF
CRLF
;

static const char response_lua_chunked_end[] = "0\r\n\r\n";

static const char co_default_entry[] = 
"local args = {...};" CRLF
"local cnn = args[1];" CRLF
"local filepath = args[2];" CRLF
"local p = '(.+)%?(.+)';" CRLF
"local _, params = string.match(args[3], p);" CRLF
"local function _p2t(p)" CRLF
"  local t = {};" CRLF
"  if not p then" CRLF
"    return t;" CRLF
"  end" CRLF
"  local text = p .. '&';" CRLF
"  for sec in string.gmatch(text, '(.-)&') do" CRLF
"    local k, v = string.match(sec, '(.+)=(.*)');" CRLF
"    t[k] = v;" CRLF
"  end" CRLF
"  return t;" CRLF
"end" CRLF
"local t = _p2t(params);" CRLF
"local function say(msg)" CRLF
"  return mweb.say(cnn, msg, 0);" CRLF
"end" CRLF
"local function write(msg)" CRLF
"  return mweb.say(cnn, msg, 1);" CRLF
"end" CRLF
"local function wa_cb(cb, cb_data)" CRLF
"  return mweb.write_after_cb(cnn, cb, cb_data);" CRLF
"end" CRLF
"local m = {say = say, write = write, wa_cb = wa_cb, version = '1.0.0', c = cnn};" CRLF
"local co = coroutine.create(function(script, lib, ctx)" CRLF
"  local run = coroutine.running();" CRLF
"  if not _G.libmweb_cos then" CRLF
"    _G.libmweb_cos = {};" CRLF
"  end" CRLF
"  table.insert(_G.libmweb_cos, run);" CRLF
"  " CRLF
"  local f = loadfile(script)" CRLF
"  if f then" CRLF
"    f(lib, ctx);" CRLF
"  end" CRLF
"  mweb.close(lib.c);" CRLF
"  for k, v in pairs(_G.libmweb_cos) do" CRLF
"    if v == run then" CRLF
"       table.remove(_G.libmweb_cos, k);" CRLF
"       break;" CRLF
"    end" CRLF
"  end" CRLF
"end);" CRLF
"coroutine.resume(co, filepath, m, t);" CRLF
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
    mweb_response_lua_context_t *context = mweb_alloc(sizeof(mweb_response_lua_context_t));
    if(context){
        MWSTRING_COPY_CSTRING(context->res, response_lua_begin, strlen(response_lua_begin));
        context->co = NULL;
        context->wa_func_ref = context->wa_data_ref = LUA_REFNIL;
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
    if(context->co){
        luaL_unref(context->co, LUA_REGISTRYINDEX, context->wa_func_ref);
        luaL_unref(context->co, LUA_REGISTRYINDEX, context->wa_data_ref);
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
        //NULL;
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
        response->buf = uv_buf_init((char*)context->res.base, context->res.len-1);
        if(uv_write(&response->req, (uv_stream_t*)response->stream, &response->buf, 1, mweb_response_after_write_cb)){
            ERR("Send response failed\n");
        }
    }else{
        return mweb_http_response_404(cnn, response_send_complete_cb);
    }
    return response;   
}

/*
    lua chunked req
*/
typedef struct {
    uv_write_t req;
    uv_buf_t buf;
    size_t data_len;
}mweb_lua_chunked_req_t;

static void mweb_lua_chunked_after_write_cb(uv_write_t* req, int status) {
    mweb_lua_chunked_req_t* wr = (mweb_lua_chunked_req_t*) req;
    mweb_http_response_t *response = (mweb_http_response_t *)wr->req.data;
    mweb_response_lua_context_t *context = (mweb_response_lua_context_t*)response->context;

    if(context){
        if(context->co){
            lua_rawgeti(context->co, LUA_REGISTRYINDEX, context->wa_func_ref);
            lua_rawgeti(context->co, LUA_REGISTRYINDEX, context->wa_data_ref);
            lua_pushinteger(context->co, status);
            lua_pushinteger(context->co, wr->data_len);
            lua_call(context->co, 3, 0);
        }
    }
    mweb_free(wr->buf.base);
    mweb_free(wr);

    if (status == 0)
        return;

    ERR("uv_write error: %s\n", uv_strerror(status));

    if (status == UV_ECANCELED)
        return;
}

static void mweb_lua_chunked_close_after_write_cb(uv_write_t* req, int status) {
    mweb_lua_chunked_req_t* wr = (mweb_lua_chunked_req_t*) req;
    mweb_http_response_t *response = (mweb_http_response_t *)wr->req.data;
    mweb_free(wr);

    mweb_response_lua_context_t *context = (mweb_response_lua_context_t *)response->context;
    mweb_lua_context_destory(context);
    response->context = NULL;
    response->response_send_complete_cb(response->connection, status);
}

static int l_mweb_say(lua_State *L){
    mweb_lua_chunked_req_t *wr;
    mweb_http_connection_t *cnn = (mweb_http_connection_t *)lua_topointer(L, 1);        
    mweb_http_response_t *response = cnn->response;
    size_t msg_len;
    const char *msg = luaL_checklstring(L, 2, &msg_len);
    int mode = 0; /*mode = zero, mean chunked encoding protocol, otherwise mean raw data.*/
    if(uv_is_closing((uv_handle_t*)cnn->stream)){
        mweb_response_lua_context_t *context = (mweb_response_lua_context_t*)response->context;
        if(context){
            if(context->co){
                lua_rawgeti(context->co, LUA_REGISTRYINDEX, context->wa_func_ref);
                lua_rawgeti(context->co, LUA_REGISTRYINDEX, context->wa_data_ref);
                lua_pushinteger(context->co, -1);
                lua_pushinteger(context->co, (int)msg_len);
                lua_call(context->co, 3, 0);
            }
        }
        return 0;
    }
    if(lua_isnumber(L, 3)){
        mode = luaL_checkint(L, 3);
    }
    wr = mweb_alloc(sizeof(mweb_lua_chunked_req_t));
    if(wr){
        size_t packet_len;
        char *base;
        if(mode){
            base = (char*)mweb_alloc(msg_len);
            memcpy(base, msg, msg_len);
            packet_len = msg_len;
        }else{
            size_t header_len, tail_len = 2;
            char header[32];
            const char *tail = "\r\n";
            sprintf(header, "%zX\r\n", msg_len);
            header_len = strlen(header);
            base = (char*)mweb_alloc(msg_len+256);
            memcpy(base, header, header_len);
            memcpy(base+header_len, msg, msg_len);
            memcpy(base + header_len + msg_len, tail, tail_len);
            packet_len = header_len + msg_len + tail_len;
        }
        wr->req.data = response;        
        wr->buf = uv_buf_init(base, packet_len);
        wr->data_len = msg_len;
        uv_write(&wr->req, (uv_stream_t*)response->stream, &wr->buf, 1, mweb_lua_chunked_after_write_cb);
    }else{
        ERR("mweb_alloc mweb_lua_chunked_req_t failed\n");
    }
    return 0;
}

static int l_mweb_close(lua_State *L){
    mweb_lua_chunked_req_t *wr;
    mweb_http_connection_t *cnn = (mweb_http_connection_t *)lua_topointer(L, 1);
    mweb_http_response_t *response = cnn->response;

    if(uv_is_closing((uv_handle_t*)cnn->stream)){
        mweb_response_lua_context_t *context = (mweb_response_lua_context_t *)response->context;
        mweb_lua_context_destory(context);
        response->context = NULL;
        response->response_send_complete_cb(response->connection, -1);
        return 0;
    }

    wr = mweb_alloc(sizeof(mweb_lua_chunked_req_t));
    if(wr){
        wr->req.data = response;
        wr->buf = uv_buf_init((char*)response_lua_chunked_end, sizeof(response_lua_chunked_end));
        uv_write(&wr->req, (uv_stream_t*)response->stream, &wr->buf, 1, mweb_lua_chunked_close_after_write_cb);
    }else{
        ERR("mweb_alloc mweb_lua_chunked_req_t failed\n");
    }
    return 0;
}

static int l_mweb_write_after_cb(lua_State *L){
    mweb_http_connection_t *cnn = (mweb_http_connection_t *)lua_topointer(L, 1);        
    mweb_http_response_t *response = cnn->response;
    mweb_response_lua_context_t *ctx = (mweb_response_lua_context_t*)response->context;
    if(ctx->co){
        luaL_unref(ctx->co, LUA_REGISTRYINDEX, ctx->wa_func_ref);
        luaL_unref(ctx->co, LUA_REGISTRYINDEX, ctx->wa_data_ref);
    }
    ctx->co = cnn->server->L;
    ctx->wa_func_ref = ctx->wa_data_ref = LUA_REFNIL;
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    ctx->wa_func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushvalue(L, 3);
    ctx->wa_data_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pushinteger(L, 0);
    return 0;
}

static struct  luaL_Reg mweb_methods[] = {
    {"say", l_mweb_say},
    {"close", l_mweb_close},
    {"write_after_cb", l_mweb_write_after_cb},
    {NULL, NULL},
};

int luaopen_mweb(lua_State* L){
    luaL_register(L, "mweb", mweb_methods);
    return 1;
}

int mweb_http_response(mweb_http_connection_t *cnn, mweb_http_response_send_complete_cb response_send_complete_cb){
    char filepath[8192];
    const char *root = cnn->server->wwwroot;
    const char *url = cnn->request->url.base;
    if(strcmp(url, "/") == 0){
        url = "/index.html";
    }
    const char *tk = strstr(url, "?");
    if(tk){
        char dp[4096];
        strncpy(dp, url, 4095);
        dp[tk-url] = 0;
        sprintf(filepath, "%s%s", root, dp);
    }else{
        sprintf(filepath, "%s%s", root, url);
    }
    if(filepath[strlen(filepath) - 1] == '/'){
        cnn->response = mweb_http_response_404(cnn, response_send_complete_cb);
    }else if(strcmp(filepath + strlen(filepath) - 4, ".lua") == 0){
        cnn->response = mweb_http_response_lua(cnn, response_send_complete_cb, filepath);
        if(cnn->server->L){
            lua_State *L = cnn->server->L;
            int error = -1;
            error = luaL_loadstring(L, co_default_entry);
            if(error){
                ERR("%s", lua_tostring(L, -1));
                lua_pop(L, 1);
            }else{
                lua_pushlightuserdata(L, cnn);
                lua_pushstring(L, filepath);
                lua_pushstring(L, cnn->request->url.base);
                error = lua_pcall(L, 3, 0, 0);
                if(error){
                    ERR("%s", lua_tostring(L, -1));
                    lua_pop(L, 1);
                }
            }
        }
    }else{
        cnn->response = mweb_http_response_file(cnn, response_send_complete_cb, filepath);
    }

    return 0;
}

void mweb_http_response_destory(mweb_http_response_t* response){
    mweb_free(response);
}
