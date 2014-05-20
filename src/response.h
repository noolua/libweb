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
#include "mweb_types.h"

int luaopen_mweb(lua_State* L);
int mweb_http_response(mweb_http_connection_t *cnn, mweb_http_response_send_complete_cb response_senc_complete_cb);
void mweb_http_response_destory(mweb_http_response_t* response);

#endif
