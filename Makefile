
all:  test fileBuffers queryResult

test: tests/tests.cpp
	g++ -g -std=c++14 $? -o $@ -DDEBUG_CACHES -I./include

fileBuffers: examples/FileBuffers.cpp
	g++ -Ofast -std=c++14 $? -o $@ -I./include

queryResult: examples/QueryResult.cpp
	g++ -Ofast -std=c++14 $? -o $@ -I./include

clean:
	rm test fileBuffers queryResult *.bin
