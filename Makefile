# script para compilar os programas

all: directories upload findrec seek

directories:
	mkdir -p objects
	mkdir -p datafiles
	mkdir -p indexes

findrec:  objects/records.o objects/util.o objects/findrec.o
	g++ -std=c++11  -o findrec objects/records.o objects/btree.o objects/util.o objects/findrec.o

upload: objects/records.o  objects/btree.o objects/util.o objects/upload.o
	g++ -std=c++11 -o upload  objects/records.o   objects/btree.o objects/util.o objects/upload.o

objects/findrec.o: codigosFonte/findrec.cpp 
	g++ -std=c++11 -o objects/findrec.o -c codigosFonte/findrec.cpp

objects/upload.o: codigosFonte/upload.cpp 
	g++ -std=c++11 -o objects/upload.o -c codigosFonte/upload.cpp

objects/btree.o: codigosFonte/btree.cpp cabecalhos/btree.hpp cabecalhos/util.hpp
	g++ -std=c++11 -o objects/btree.o -c codigosFonte/btree.cpp

objects/records.o: codigosFonte/records.cpp cabecalhos/records.hpp
	g++ -std=c++11 -o objects/records.o -c codigosFonte/records.cpp

objects/util.o: codigosFonte/util.cpp cabecalhos/util.hpp
	g++ -std=c++11 -o objects/util.o -c codigosFonte/util.cpp

clean:
	rm -f objects/*.o
	rm -f datafiles/*
	rm -f upload
	rm -f findrec
