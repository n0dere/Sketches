CC ?= cc
INCLUDES = -I../../serial
SRCS = ./changer.c ../../serial/unix_serial.c
GTK3 = $$(pkg-config --cflags gtk+-3.0) $$(pkg-config --libs gtk+-3.0)
OUT = changer.out

all:
	$(CC) $(INCLUDES) -g $(GTK3) $(SRCS) -o $(OUT)

clean:
	rm -f $(OUT)