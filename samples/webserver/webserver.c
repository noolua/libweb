//
//  main.c
//  ddpush-daemon
//
//  Created by rockee on 13-9-29.
//  Copyright (c) 2013年 coco-hub.com. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mweb.h"

#define NULL_DEV "/dev/null"
#define WWW_ROOT_DEFAULT    "/var/miniweb"
#define LOCKFILE "/var/run/miniweb.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

int main(int argc, const char** argv) {
    int ret = 0;
    uv_loop_t *loop = uv_default_loop();
    ret = mweb_startup(loop, "127.0.0.1", 3000);
    if (ret < 0) {
        ERR("server startup failed: %d\n", ret);
        return -1;
    }
    
    /* http_service_run */
    uv_run(loop, UV_RUN_DEFAULT);
    
    /* cleanup */
    ret = mweb_cleanup();

    return ret;
}


