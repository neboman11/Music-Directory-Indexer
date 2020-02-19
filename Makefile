Music-Directory-Indexer: main.o
	g++ main.o -o Music-Directory-Indexer

main.o: main.cpp
	g++ main.cpp -c -Wall -std=c++17
