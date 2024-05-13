all : bill.o source.o account.o building.o

source.o: 	source.cpp
			g++ source.cpp -o source

account.o: account.cpp
			g++ account.cpp -o account.o

building.o: building.cpp
			g++ building.cpp -o building.o

bill.o: bill.cpp
		g++ bill.cpp -o bill.o

clean: 
	-rm *.fifo *.o source