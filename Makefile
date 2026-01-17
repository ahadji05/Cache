
all:  tests.exe exampleFileBuffers.exe

tests.exe: tests.cpp
	g++ -g -std=c++14 $? -o $@ -DDEBUG_CACHES

exampleFileBuffers.exe: exampleFileBuffers.cpp
	g++ -Ofast -std=c++14 $? -o $@ 

clean:
	rm tests.exe exampleFileBuffers.exe