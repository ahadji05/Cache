
main.exe: main.cpp
	g++ -Ofast $? -o $@ 
#-DDEBUG_CACHES

clean:
	rm main.exe
