final: NNDFULL.o  
	g++ -o2 -o demo NNDFULL.o -std=c++11

NNDFULL.o: NNDFULL.cc
	g++ -c NNDFULL.cc -std=c++11

clean:
#Linux
	rm -f *.o
	rm -f *.gch