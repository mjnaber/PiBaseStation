LIBS=-lcurl -lxml2

sink: sink.c
	gcc -o sink sink.c $(LIBS)

