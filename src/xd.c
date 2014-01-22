#include <stdio.h>
#include <stdlib.h>

#include "mweb.h"

http_parser* web_parser_create(int a){
    http_parser *one =malloc(sizeof(http_parser));
    http_parser_init(one, HTTP_REQUEST);
    return one;
}

void web_parser_destory(http_parser* one){
    if(one)
        free(one);
}

