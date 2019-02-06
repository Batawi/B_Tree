all: main.o
	g++ -std=c++11 main.o -Wall -o out 
	./out

main.o: main.cpp
	g++ -std=c++11 main.cpp -Wall -c -o main.o

clean:
	rm -f *.o out

#just lunch output file
g:
	./out
