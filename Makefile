CC=gcc
CFLAGS=-g -Wall
LFLAGS=-lm -lrt -pthread
TARGET1=hw4
TARGET2=hwdiff
TARGET3=showdata
OBJS1=$(TARGET1).o wrapper.o
OBJS2=$(TARGET2).o wrapper.o
OBJS3=$(TARGET3).o wrapper.o

all: $(TARGET1) $(TARGET2) $(TARGET3)

$(TARGET1): $(OBJS1)
	$(CC) -o $@ $(OBJS1) $(LFLAGS)
$(TARGET2): $(OBJS2)
	$(CC) -o $@ $(OBJS2) $(LFLAGS)
$(TARGET3): $(OBJS3)
	$(CC) -o $@ $(OBJS3) $(LFLAGS)
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET1).o: wrapper.h
$(TARGET2).o: wrapper.h
$(TARGET3).o: wrapper.h
wrapper.o: wrapper.h

clean:
	rm $(TARGET1) $(TARGET2) $(TARGET3) *.o

test: $(TARGET1) $(TARGET2)
	bash test.sh $(ARG)
