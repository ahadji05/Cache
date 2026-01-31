
all:  tests.exe exampleFileBuffers.exe QueryResult.exe

tests.exe: tests/tests.cpp
	g++ -g -std=c++14 $? -o $@ -DDEBUG_CACHES -I./include/kash

exampleFileBuffers.exe: examples/exampleFileBuffers.cpp
	g++ -Ofast -std=c++14 $? -o $@ -I./include/kash

QueryResult.exe: examples/QueryResult.cpp
	g++ -Ofast -std=c++14 $? -o $@ -I./include/kash

clean:
	rm tests.exe exampleFileBuffers.exe QueryResult.exe *.bin
