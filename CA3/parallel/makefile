all: tbmp.o

tbmp.o: bmp-thread.cpp flip.cpp blur.cpp purp.cpp lib.hpp
		g++ -std=c++11 bmp-thread.cpp -lpthread -o ImageFilters.out

clean: 
		-rm *.o *.out *.bmp