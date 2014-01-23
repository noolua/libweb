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
int mweb_startup(uv_loop_t *loop, const char *address, int port);
int mweb_is_running();
int mweb_cleanup();


#endif // miniweb_mweb_h