
OBJS=gputest_mssva.o

CFLAGS=-O2 -I/opt/mssva/include

LD_FLAGS=-L/opt/mssva/lib

LIBS=-lva -lva-drm

all: mss_test

mss_test: $(OBJS)
	$(CC) -o mss_test $(OBJS) $(LD_FLAGS) $(LIBS)

clean:
	rm -f mss_test $(OBJS)
