.PHONY: clean format

CPPFLAGS = -std=c++11 -O3 -Wall

all: gen server

clean:
	rm -f gen
	rm -f *.csv generator/*.csv
	rm -f *.dsl generator/*.dsl
	rm -f *.exp generator/*.exp
	rm -f server
	rm -rf data
	rm -f *.res

gen: generator/generator.go
	cd generator && go build . && mv generator ../gen

server: server.cpp server.h
	clang++ -o server $(CPPFLAGS) server.cpp

format:
	clang-format -i *.cpp *.h
