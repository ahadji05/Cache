
main.exe: main.cpp
	g++ -g -std=c++14 $? -o $@ -DDEBUG_CACHES

clean:
	rm main.exe
