all: myclient myserver

myclient: src/myclient.cpp src/myhelper/myhelper.o src/myhelper/myhelper.h src/mysocket/mysocket.o src/mysocket/mysocket.h 
	g++ -std=c++0x -o myclient src/myhelper/myhelper.o src/mysocket/mysocket.o src/myclient.cpp -Wall

myserver: src/myserver.cpp src/myhelper/myhelper.o src/myhelper/myhelper.h src/mysocket/mysocket.o src/mysocket/mysocket.h
	g++ -std=c++0x -o myserver src/myhelper/myhelper.o src/mysocket/mysocket.o src/myserver.cpp -Wall -pthread

clean:
	rm -f myserver myclient src/mysocket/mysocket.o src/myhelper/myhelper.o