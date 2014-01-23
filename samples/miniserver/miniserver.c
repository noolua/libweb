#include <stdio.h>
#include "mweb.h"

int main(){
    const char *address = "127.0.0.1";
    int port = 3000;
    uv_loop_t *loop = uv_default_loop();
    mweb_startup(loop, address, port);
    LOG("server bind: '%s', listen: %d\n", address, port);

    uv_run(loop, UV_RUN_DEFAULT);
    LOG("server cleanup\n");
    mweb_cleanup();
    return 0;
}
