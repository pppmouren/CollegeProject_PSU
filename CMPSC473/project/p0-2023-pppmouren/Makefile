out: print.c extvar.o
	cc -g print.c extvar.o -o out

CFLAGS = -std=gnu99 
SOURCES = print.c extvar.o
OUT = out

default:
	gcc $(CFLAGS) $(SOURCES) -o $(OUT)
debug:
	gcc -g $(CFLAGS) $(SOURCES) -o $(OUT)
all:
	gcc $(SOURCES) -o $(OUT)
clean:
	rm -f $(OUT)
