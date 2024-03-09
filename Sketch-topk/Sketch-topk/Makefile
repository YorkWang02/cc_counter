CPPFLAGS = -Wall -O3 -std=c++14 -lm -w -mcmodel=medium -g
PROGRAMS = cuckoo 

all: $(PROGRAMS)

cuckoo:main.cpp \
	BOBHASH32.h BOBHASH64.h CuckooCounter.h CuckooCounter3.h CuckooCounter31.h CuckooCounter32.h CuckooCounter33.h heavykeeper.h LossyCounting.h params.h spacesaving.h ssummary.h ASketch.h MVSketch.h NitroSketch.h ElasticSketch.h CMSketch.h HyperUss.h WavingSketch.h LossyStrategy.h DASketch.h
	g++ -o cuckoo main.cpp $(CPPFLAGS)

clean:
	rm -f *.o $(PROGRAMS)
