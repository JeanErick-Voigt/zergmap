CFLAGS=-Wall -Wextra -Wpedantic -Wwrite-strings -Wstack-usage=1024 -Wfloat-equal -Waggregate-return -Winline

all: zergMap

zergMap: zergRedone1.c
	gcc $(CFLAGS) zergRedone1.c -o zergMap -lm

debug: CFLAGS += -g
debug: all -lm

profile: CFLAGS += -pg 
profile: all -lm

clean:
	rm -f  zergMap
