CPPFLAGS = -Wall -O3 -std=c++11 -lm -w -mcmodel=medium -g
PROGRAMS = main 

all: $(PROGRAMS)

main:main.cpp \
	BOBHASH32.h BOBHASH64.h CuckooCounter.h heavykeeper.h LossyCounting.h params.h spacesaving.h ssummary.h ASketch.h MVSketch.h NitroSketch.h ElasticSketch.h
	g++ -o cuckoo main.cpp $(CPPFLAGS)

clean:
	rm -f *.o $(PROGRAMS)
