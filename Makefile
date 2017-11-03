CXX=g++
CXX_FLAGS=-Wall -std=c++14 -O2

all: cdrone

OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp)) json/jsoncpp.o

%.cpp: %.h

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -c $^ -o $@

json/jsoncpp.o: json/jsoncpp.cpp
	$(CXX) $(CXX_FLAGS) -I. -c $^ -o $@
	
cdrone: $(OBJS)
	$(CXX) $(CXX_FLAGS) $^ -o $@

clean:
	@rm -f cdrone *.o
