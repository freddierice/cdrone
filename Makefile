CXX=g++
CXX_FLAGS=-O2

all: cdrone

cdrone: cdrone.cpp
	$(CXX) $(CXX_FLAGS) cdrone.cpp -o cdrone

clean:
	@rm -f cdrone
