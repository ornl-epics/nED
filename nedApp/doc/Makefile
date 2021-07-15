.PHONY: clean html pdf generate

all: html

install: all

pdf: Doxyfile generate
	(cd latex/; make pdf > /dev/null)

html: Doxyfile generate
	doxygen >/dev/null

clean:
	rm -fR html/ latex/ tmp/

generate: $(wildcard ../src/*.cpp)
	mkdir -p tmp/
	./export2doxy.py -v -o tmp/ ../src/*cpp
