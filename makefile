sinterp: evaluate.o parser_type.o main.o
	g++ -o sinterp main.o evaluate.o parser_type.o

evaluate.o : src/evaluate.cpp
	g++ -std=c++17 -c src/evaluate.cpp

main.o : src/main.cpp
	g++ -std=c++17 -c src/main.cpp

parser_type.o : src/parser_type.cpp
	g++ -std=c++17 -c src/parser_type.cpp
