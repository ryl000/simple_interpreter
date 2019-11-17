sinterp: main.o
	g++ -o sinterp main.o

main.o : src/main.cpp
	g++ -std=c++17 -c src/main.cpp
