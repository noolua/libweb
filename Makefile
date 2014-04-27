TARGET=mweb
LIBNAME=$(addprefix lib, $(TARGET)).a

LIBOBJS+=./deps/http-parser/http_parser.o
LIBOBJS+=./src/connection.o
LIBOBJS+=./src/response.o
LIBOBJS+=./src/request.o
LIBOBJS+=./src/server.o

PREFIX=/usr/local
CFLAGS+=-std=gnu99 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -Wall -O2 -fPIC -I./include -I./src -I./deps/http-parser -I$(HOME)/usr/local/include 
LDFLAGS= -L$(HOME)/usr/local/lib
CC=gcc
AR=ar
RANLIB=ranlib
ARFLAGS=rcu
STRIP=strip
LIBS+=uv lua pthread m


all:./deps/http-parser samples

./deps/http-parser:
	git clone --depth 1 https://github.com/joyent/http-parser.git ./deps/http-parser
	@mkdir -p ./lib
$(LIBNAME):$(LIBOBJS)
	@$(AR) $(ARFLAGS) ./lib/$(LIBNAME) $(LIBOBJS)	
samples:$(LIBNAME) miniserver webserver

miniserver:$(LIBNAME)
	@echo "Compiling $@"
	@$(CC) -I./include $(CFLAGS) -I./src -c ./samples/miniserver/miniserver.c -o ./samples/miniserver/miniserver.o
	@$(CC) -I./include $(CFLAGS) ./samples/miniserver/miniserver.o -L./lib $(LDFLAGS) -l${TARGET} $(addprefix -l,$(LIBS)) -o ./samples/miniserver/$@
	@$(STRIP) ./samples/miniserver/$@
webserver:$(LIBNAME)
	@echo "Compiling $@"
	@$(CC) -I./include $(CFLAGS) -I./src -c ./samples/webserver/webserver.c -o ./samples/webserver/webserver.o
	@$(CC) -I./include $(CFLAGS) ./samples/webserver/webserver.o -L./lib $(LDFLAGS) -l${TARGET} $(addprefix -l,$(LIBS)) -o ./samples/webserver/$@
	@$(STRIP) ./samples/webserver/$@
install:
	cp ./include/*.h ${PREFIX}/include/
	cp ./lib/*.a ${PREFIX}/lib/
uninstall:
	@echo "rm '$(TARGET)' form '${PREFIX}/bin'"
	@rm -f ${PREFIX}/bin/$(TARGET)
	@rm -f ${PREFIX}/lib/$(LIBNAME)
touch:
	touch $(SRCS) 
distclean:clean
	@rm -rf ./deps/http-parser
clean:
	@echo "Clean ..."
	@rm -f ./deps/http-parser/*.o
	@rm -f ./samples/webserver/*.o ./samples/webserver/webserver
	@rm -f ./samples/miniserver/*.o ./samples/miniserver/miniserver
	@rm -f ./$(TARGET)
	@rm -f ./lib/*.a
	@rm -f ./src/*.o
	@rm -f *~ */*~

.SECONDARY    :%.o 
# Compiling to object files.
%.o:%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: test

