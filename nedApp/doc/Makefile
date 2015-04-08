.PHONY: clean html pdf mkdirtmp

all: mkdirtmp html
install: all

pdf:
	(cd latex/; make pdf > /dev/null)

html: Doxyfile $(wildcard ../src/*.cpp)
	./export2doxy.py -v -o tmp/ ../src/*cpp
	doxygen >/dev/null

clean:
	rm -fR html/ latex/ tmp/

mkdirtmp:
	mkdir -p tmp/

