
main.exe: main.cpp
	g++ -std=c++14 -Ofast $? -o $@ -DDEBUG_CACHES

clean:
	rm main.exe
