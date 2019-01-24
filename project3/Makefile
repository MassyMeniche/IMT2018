prog: main.o binomialtree.o
	g++ -o prog main.o binomialtree.o -lQuantLib
	
binomialtree.o: binomialtree.cpp 
	g++ -o binomialtree.o -c binomialtree.cpp 

main.o: main.cpp binomialtree.hpp binomialengine.hpp
	g++ -o main.o -c main.cpp