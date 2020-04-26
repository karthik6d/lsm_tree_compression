.PHONY: clean generator

all: generator

clean:
	rm -f gen
	rm -f *.csv generator/*.csv
	rm -f *.dsl generator/*.dsl
	rm -f *.exp generator/*.exp

generator: generator/generator.go
	cd generator && go build . && mv generator ../gen
