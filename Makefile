TARGET=eweb
LIBNAME=$(addprefix lib, $(TARGET)).a

LIBOBJS+=./deps/http-parser/http_parser.o
LIBOBJS+=./src/xd.o

PREFIX=/usr/local
CFLAGS+=-std=gnu99 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -Wall -O2 -fPIC -I./include -I./src -I./deps/http-parser -I$(HOME)/usr/local/include 
LDFLAGS= -L$(HOME)/usr/local/lib
CC=gcc
AR=ar
RANLIB=ranlib
ARFLAGS=rcu
STRIP=strip
LIBS+=lua pthread m


all:./deps/http-parser samples

./deps/http-parser:
	git clone --depth 1 https://github.com/joyent/http-parser.git ./deps/http-parser
	@mkdir -p ./lib
$(LIBNAME):$(LIBOBJS)
	@$(AR) $(ARFLAGS) ./lib/$(LIBNAME) $(LIBOBJS)
samples:$(LIBNAME)
	@echo "Compiling $@"
	@$(CC) -I./include $(CFLAGS) -I./src -c ./samples/src/main.c -o ./samples/src/main.o
	@$(CC) -I./include $(CFLAGS) ./samples/src/main.o -L./lib $(LDFLAGS) $(addprefix -l,$(LIBS)) -l${TARGET} -o ./samples/$@	
	@$(STRIP) ./samples/$@
install:
	@echo "intstall '$(TARGET)' -> ${PREFIX}/bin/"
	@cp ./$(TARGET) ${PREFIX}/bin/
	@cp ./include/*.h ${PREFIX}/include/
	@cp ./lib/*.a ${PREFIX}/lib/
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
	@rm -f ./samples/src/*.o
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

