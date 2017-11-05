PREFIX=armv8-rpi3-linux-gnueabihf-
CXX=$(PREFIX)g++
CXX_FLAGS=-Wall -std=c++14 -O2

all: cdrone

OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp)) json/jsoncpp.o

%.cpp: %.h

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -c $^ -o $@

json/jsoncpp.o: json/jsoncpp.cpp
	$(CXX) $(CXX_FLAGS) -I. -c $^ -o $@
	
cdrone: $(OBJS)
	$(CXX) $(CXX_FLAGS) $^ -o $@ -lpthread

push: cdrone
	scp cdrone drone-home:

run: cdrone
	scp cdrone drone-home:
	ssh drone-home ./cdrone

clean:
	@rm -f cdrone *.o json/jsoncpp.o
