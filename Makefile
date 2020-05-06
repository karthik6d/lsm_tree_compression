.PHONY: clean format profile

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
	rm -f perf.data*

gen: generator/generator.go
	cd generator && go build . && mv generator ../gen

server: server.cpp server.h
	g++ -o server $(CPPFLAGS) server.cpp quicksort.cpp

format:
	clang-format -i *.cpp *.h

profile: server
	sudo perf record --call-graph dwarf ./server queries.dsl
	sudo perf report
