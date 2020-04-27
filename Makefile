.PHONY: clean generator

CPPFLAGS = -std=c++11 -O3 -Wall

all: generator server

clean:
	rm -f gen
	rm -f *.csv generator/*.csv
	rm -f *.dsl generator/*.dsl
	rm -f *.exp generator/*.exp
	rm -f server
	rm -rf data
	rm -f *.res

generator: generator/generator.go
	cd generator && go build . && mv generator ../gen

server: server.cpp server.h
	clang++ -o server $(CPPFLAGS) server.cpp
