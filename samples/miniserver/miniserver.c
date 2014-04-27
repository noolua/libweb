#include <stdio.h>
#include "mweb.h"

int main(){
    const char *address = "0.0.0.0";
    //const char *wwwroot = "/Users/rockee/oschina/libweb-0.0.1/samples/www";
    const char *wwwroot = "/Users/rockee/oschina/ddpush-daemon-0.0.1/www/mbox";
    int cacheoff = 1;
    int port = 3000;
    uv_loop_t *loop = uv_default_loop();
    mweb_startup(loop, address, port, wwwroot, cacheoff, NULL, NULL);
    LOG("server www_root: '%s', cacheoff: %d\n", wwwroot, cacheoff);
    LOG("server bind: '%s', listen: %d\n", address, port);

    uv_run(loop, UV_RUN_DEFAULT);
    LOG("server cleanup\n");
    mweb_cleanup();
    return 0;
}
