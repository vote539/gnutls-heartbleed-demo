# CC = g++
# CFLAGS = -g -Wall -DHAVE_CONFIG_H -I../gnutls/doc/examples -I../gnutls -I../gnutls/lib/includes -I../gnutls/extra/includes -I../gnutls/gl -D_GL_NO_LARGE_FILES -DNO_LIBCURL
TARGET = demo
# LIBS = ../gnutls/lib/libgnutlsxx.la ../gnutls/doc/examples/libexamples.la ../gnutls/lib/libgnutls.la ../gnutls/gl/libgnu.la
GNUTLS_DIR = ../gnutls

all: $(TARGET)-c
run: -run-$(TARGET)-c

$(TARGET)-c.o: $(TARGET).c
	gcc -Wall -g -O2 -o $(TARGET)-c.o -c $(TARGET).c
$(TARGET)-c: $(TARGET)-c.o tcp.o heartbleed.o
	$(GNUTLS_DIR)/libtool --silent --tag=CC  --mode=link gcc -Wall -g -O2 -no-install -o $(TARGET)-c $(TARGET)-c.o tcp.o heartbleed.o -lgnutls $(GNUTLS_DIR)/lib/libgnutls.la $(GNUTLS_DIR)/gl/libgnu.la
-run-$(TARGET)-c: $(TARGET)-c
	./$(TARGET)-c

heartbleed.o: heartbleed.c
	gcc -Wall -g -O2 -o heartbleed.o -c heartbleed.c
tcp.o: tcp.c
	gcc -Wall -c -o tcp.o tcp.c

clean:
	$(RM) tcp.o $(TARGET)-*
