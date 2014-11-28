.PHONY: clean pdf mkdirtmp

all: mkdirtmp doc pdf
install: all

pdf:
	(cd latex/; make pdf > /dev/null)

doc: Doxyfile $(wildcard ../src/*.cpp)
	./export2doxy.py -v -o tmp/ ../src/*cpp
	doxygen >/dev/null

clean:
	rm -fR html/ latex/ tmp/

mkdirtmp:
	mkdir -p tmp/

