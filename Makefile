Music-Directory-Indexer: main.o
	g++ main.o -o Music-Directory-Indexer

main.o: main.cpp
	g++ main.cpp -c -Wall -std=c++17

debug: main.cpp
	g++ main.cpp -g -Wall -std=c++17 -o debug
