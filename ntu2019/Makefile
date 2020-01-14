CC = g++

CCFLAGS = -O3 -W
CFLAGS = $(CCFLAGS) -I../libjbig -L../libjbig/

all: hw1.cpp
	$(CC) $(CFLAGS) -o hw -DTEST_CODEC hw1.cpp ../libjbig/jbig.c \
	../libjbig/jbig_tab.o
