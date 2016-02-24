final: mytry.o  
	g++ -o demo mytry.o -std=c++11

mytry.o: mytry.cc
	g++ -c mytry.cc -std=c++11

clean:
#Linux
	rm -f *.o
	rm -f *.gch