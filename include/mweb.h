//
//  mweb.h
//  miniweb
//
//  Created by rockee on 14-1-22.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//
#ifndef miniweb_mweb_h
#define miniweb_mweb_h

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

extern int MWEB_QUIET;
extern int MWEB_SYSLOG;

void mweb_quiet_syslog(int quiet, int syslog);
int mweb_startup(uv_loop_t *loop, const char *address, int port);
int mweb_cleanup();


#endif // miniweb_mweb_h