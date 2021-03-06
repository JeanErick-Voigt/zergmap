CFLAGS=-Wall -Wextra -Wpedantic -Wwrite-strings -Wstack-usage=1024 -Wfloat-equal -Waggregate-return -Winline

all: zergMap

zergMap: zergmap.c
	gcc $(CFLAGS) zergmap.c -o zergMap -lm

debug: CFLAGS += -g
debug: all -lm

profile: CFLAGS += -pg 
profile: all -lm

clean:
	rm -f  zergMap
