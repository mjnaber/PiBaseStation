LIBS = -lcurl -lxml2 -lwiringPi

INCLUDE = -I/usr/include -I/usr/include/libxml2

FLAGS = -Wall

sink: sink.c
	gcc -o sink sink.c $(LIBS) $(INCLUDE) $(FLAGS)

powermon: powermon.c
	gcc -o powermon powermon.c $(LIBS) $(INCLUDE) $(FLAGS)
