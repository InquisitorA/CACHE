compile: main.cpp
	g++ -std=c++11 main.cpp -o cache_simulate

run: main.cpp
	./cache_simulate 64 1024 2 65536 8 trace1.txt

clean:
	rm -rf *.o final