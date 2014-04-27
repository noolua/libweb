//
//  mweb.h
//  miniweb
//
//  Created by rockee on 14-1-22.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//
#ifndef miniweb_mweb_h
#define miniweb_mweb_h

#include <stdio.h>

#ifndef HAVE_ALLOC_POOL
    #include <stdlib.h>
    #define mweb_alloc(size)    malloc(size)
    #define mweb_free(ptr)      free(ptr)
#else
    #define mweb_alloc(size)  NULL
    #define mweb_free(ptr)
#endif

#include <syslog.h>
#include <uv.h>

#include "http_parser.h"

#define LOG(...)                                            \
    do {                                                    \
        if (!MWEB_QUIET) fprintf(stdout, __VA_ARGS__);      \
        if (MWEB_SYSLOG) syslog(LOG_INFO, __VA_ARGS__);     \
} while(0)

#define ERR(...)                                            \
    do {                                                    \
        fprintf(stderr, __VA_ARGS__);                       \
        if (MWEB_SYSLOG) syslog(LOG_ERR, __VA_ARGS__);      \
} while(0)

/* Informational 1xx */
#define HTTP_STATUS_100 "100 Continue"
#define HTTP_STATUS_101 "101 Switching Protocols"
#define HTTP_STATUS_102 "102 Processing"
/* Successful 2xx */
#define HTTP_STATUS_200 "200 OK"
#define HTTP_STATUS_201 "201 Created"
#define HTTP_STATUS_202 "202 Accepted"
#define HTTP_STATUS_203 "203 Non-Authoritative Information"
#define HTTP_STATUS_204 "204 No Content"
#define HTTP_STATUS_205 "205 Reset Content"
#define HTTP_STATUS_206 "206 Partial Content"
#define HTTP_STATUS_207 "207 Multi-Status"
/* Redirection 3xx */
#define HTTP_STATUS_300 "300 Multiple Choices"
#define HTTP_STATUS_301 "301 Moved Permanently"
#define HTTP_STATUS_302 "302 Moved Temporarily"
#define HTTP_STATUS_303 "303 See Other"
#define HTTP_STATUS_304 "304 Not Modified"
#define HTTP_STATUS_305 "305 Use Proxy"
#define HTTP_STATUS_307 "307 Temporary Redirect"
/* Client Error 4xx */
#define HTTP_STATUS_400 "400 Bad Request"
#define HTTP_STATUS_401 "401 Unauthorized"
#define HTTP_STATUS_402 "402 Payment Required"
#define HTTP_STATUS_403 "403 Forbidden"
#define HTTP_STATUS_404 "404 Not Found"
#define HTTP_STATUS_405 "405 Method Not Allowed"
#define HTTP_STATUS_406 "406 Not Acceptable"
#define HTTP_STATUS_407 "407 Proxy Authentication Required"
#define HTTP_STATUS_408 "408 Request Time-out"
#define HTTP_STATUS_409 "409 Conflict"
#define HTTP_STATUS_410 "410 Gone"
#define HTTP_STATUS_411 "411 Length Required"
#define HTTP_STATUS_412 "412 Precondition Failed"
#define HTTP_STATUS_413 "413 Request Entity Too Large"
#define HTTP_STATUS_414 "414 Request-URI Too Large"
#define HTTP_STATUS_415 "415 Unsupported Media Type"
#define HTTP_STATUS_416 "416 Requested Range Not Satisfiable"
#define HTTP_STATUS_417 "417 Expectation Failed"
#define HTTP_STATUS_418 "418 I'm a teapot"
#define HTTP_STATUS_422 "422 Unprocessable Entity"
#define HTTP_STATUS_423 "423 Locked"
#define HTTP_STATUS_424 "424 Failed Dependency"
#define HTTP_STATUS_425 "425 Unordered Collection"
#define HTTP_STATUS_426 "426 Upgrade Required"
#define HTTP_STATUS_428 "428 Precondition Required"
#define HTTP_STATUS_429 "429 Too Many Requests"
#define HTTP_STATUS_431 "431 Request Header Fields Too Large"
/* Server Error 5xx */
#define HTTP_STATUS_500 "500 Internal Server Error"
#define HTTP_STATUS_501 "501 Not Implemented"
#define HTTP_STATUS_502 "502 Bad Gateway"
#define HTTP_STATUS_503 "503 Service Unavailable"
#define HTTP_STATUS_504 "504 Gateway Time-out"
#define HTTP_STATUS_505 "505 HTTP Version Not Supported"
#define HTTP_STATUS_506 "506 Variant Also Negotiates"
#define HTTP_STATUS_507 "507 Insufficient Storage"
#define HTTP_STATUS_509 "509 Bandwidth Limit Exceeded"
#define HTTP_STATUS_510 "510 Not Extended"
#define HTTP_STATUS_511 "511 Network Authentication Required"

typedef int (*mweb_request_handle_cb)(void* req);

typedef struct mweb_string_s{
    const char* base;
    size_t len;
}mweb_string_t;

#define MWSTRING_INIT(str)  do{str.base = NULL; str.len = 0;}while(0)
#define MWSTRING_RELEASE(str) do{if(str.base){mweb_free((void*)str.base); str.base = NULL;} str.len = 0;}while(0)
#define MWSTRING_COPY_CSTRING(str, cstr, cstr_len)          \
    do{                                                     \
        char* cp_ = mweb_alloc(cstr_len+1);                 \
        memcpy(cp_, cstr, cstr_len);                        \
        cp_[cstr_len] = 0;                                  \
        str.base = cp_;                                     \
        str.len = cstr_len+1;                               \
    }while(0)


extern int MWEB_QUIET;
extern int MWEB_SYSLOG;

void mweb_quiet_syslog(int quiet, int syslog);
int mweb_startup(uv_loop_t *loop, const char *address, int port, const char* wwwroot, int cacheoff, mweb_request_handle_cb cb, void* cb_data);
int mweb_is_running();
int mweb_cleanup();


#endif // miniweb_mweb_h