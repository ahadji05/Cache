
tests.exe: tests.cpp
	g++ -g -std=c++14 $? -o $@ -DDEBUG_CACHES

clean:
	rm tests.exe