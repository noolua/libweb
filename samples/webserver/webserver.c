//
//  main.c
//  ddpush-daemon
//
//  Created by rockee on 13-9-29.
//  Copyright (c) 2013年 coco-hub.com. All rights reserved.
//

#include <stdio.h>
#include <string.h>

#include "mweb.h"

#define WWW_ROOT    "/var/webserver/www"
#define MAX_TICK 5

typedef struct timer_context_s{
    uv_timer_t timer;
    uv_loop_t *loop;
    const char *address;
    int port;
}timer_context_t;

static int tick = 0;

static void uv_timer_close_cb(uv_handle_t* handle){
    timer_context_t *ctx = handle->data;
    mweb_free(ctx);
}

static void mweb_timer_switch(uv_timer_t* timer){
    timer_context_t *ctx = (timer_context_t*)timer->data;
    if (mweb_is_running()) {
        mweb_cleanup();
        LOG("server cleanup\n");
        
        tick++;
        if(tick == MAX_TICK){
            uv_timer_stop(timer);
            uv_close((uv_handle_t*)timer, uv_timer_close_cb);
        }
    }else{
        int ret = mweb_startup(ctx->loop, ctx->address, ctx->port, WWW_ROOT, 1);
        if (ret < 0) {
            ERR("server startup failed: %d\n", ret);
        }else{
            LOG("server wwwroot: '%s'\n", WWW_ROOT);
            LOG("server startup successfull, listen: %d\n", ctx->port);
        }
    }
}

int main(int argc, const char** argv) {
    int ret = 0;
    timer_context_t *ctx = mweb_alloc(sizeof(timer_context_t));
    ctx->address = "127.0.0.1";
    ctx->port = 3000;
    ctx->timer.data = ctx;
    ctx->loop = uv_default_loop();
    uv_timer_init(ctx->loop, &ctx->timer);
    ret = uv_timer_start(&ctx->timer, mweb_timer_switch, 2000, 5000);
    
    uv_run(ctx->loop, UV_RUN_DEFAULT);
    
    return ret;
}


