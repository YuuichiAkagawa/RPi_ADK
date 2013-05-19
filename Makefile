PREFIX=/usr/local
CFLAGS=-o -I$(PREFIX)/include -I$(PREFIX)/include/libusb-1.0 -DDEBUG
LDFLAGS=-L$(PREFIX)/lib
HelloADK : HelloADK.o AOA.o
	g++ $(LDFLAGS) -o HelloADK HelloADK.o AOA.o  -lusb-1.0 -lbcm2835

AOA.o : AOA/AOA.cpp
	g++ -c $(CFLAGS) -o AOA.o AOA/AOA.cpp

HelloADK.o : HelloADK.cpp
	g++ -c $(CFLAGS) -o HelloADK.o HelloADK.cpp

clean:
	rm -f *.o HelloADK
