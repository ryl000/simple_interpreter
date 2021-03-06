sinterp.out: evaluate.o parser_type.o main.o
	g++ -g -Wall -Wextra -o sinterp.out main.o evaluate.o parser_type.o

.PHONY: clean
clean:
	rm -f *.o sinterp.out

evaluate.o : src/evaluate.cpp
	g++ -g -Wall -Wextra -std=c++17 -c src/evaluate.cpp

main.o : src/main.cpp
	g++ -g -Wall -Wextra -std=c++17 -c src/main.cpp

parser_type.o : src/parser_type.cpp
	g++ -g -Wall -Wextra -std=c++17 -c src/parser_type.cpp
