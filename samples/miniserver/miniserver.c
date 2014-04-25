#include <stdio.h>
#include "mweb.h"

int main(){
    const char *address = "0.0.0.0";
    const char *wwwroot = "/home/rockee/Downloads/libweb/samples/www";
    int cacheoff = 1;
    int port = 3000;
    uv_loop_t *loop = uv_default_loop();
    mweb_startup(loop, address, port, wwwroot, cacheoff);
    LOG("server www_root: '%s', cacheoff: %d\n", wwwroot, cacheoff);
    LOG("server bind: '%s', listen: %d\n", address, port);

    uv_run(loop, UV_RUN_DEFAULT);
    LOG("server cleanup\n");
    mweb_cleanup();
    return 0;
}
