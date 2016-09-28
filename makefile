LIBS = -lcurl -lxml2

INCLUDE = -I/usr/include -I/usr/include/libxml2

FLAGS = -Wall

sink: sink.c
	gcc -o sink sink.c $(LIBS) $(INCLUDE) $(FLAGS)

