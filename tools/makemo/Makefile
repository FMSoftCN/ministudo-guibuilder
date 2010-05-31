
xml2an: xml2an.o
	gcc $(CFLAGS) -o $@ $< $(LDFLAGS) -lxml2

xml2an.o: xml2an.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm xml2an *.o
