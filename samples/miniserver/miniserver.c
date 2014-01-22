#include <stdio.h>

#include "mweb.h"
int main(){
    http_parser *one = web_parser_create(0);    
    printf("this is sample web server\n");
    web_parser_destory(one);
    return 0;
}
