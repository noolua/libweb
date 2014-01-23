//
//  response.h
//  miniweb
//
//  Created by rockee on 14-1-22.
//  Copyright (c) 2014年 coco-hub.com. All rights reserved.
//

#ifndef miniweb_response_h
#define miniweb_response_h

#include "mweb.h"
typedef void (*mweb_http_response_send_complete_cb)(void* connection, int status);
typedef struct mweb_http_response_s mweb_http_response_t;

mweb_http_response_t *mweb_http_response_404(uv_tcp_t* stream, mweb_http_response_send_complete_cb response_send_complete_cb, void* connection);
mweb_http_response_t *mweb_http_response_file(uv_tcp_t* stream, mweb_http_response_send_complete_cb response_senc_complete_cb, void* connection, const char* filepath);
void mweb_http_response_destory(mweb_http_response_t* response);

#endif
