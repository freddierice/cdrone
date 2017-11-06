PREFIX=armv8-rpi3-linux-gnueabihf-
CXX=$(PREFIX)g++
CXX_FLAGS=-Wall -std=c++14 -O2

all: cdrone

OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp)) json/jsoncpp.o proto/io.pb.o

%.cpp: %.h

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -I. -c $^ -o $@

%.o: %.cc
	$(CXX) $(CXX_FLAGS) -I. -c $^ -o $@

cdrone: $(OBJS)
	$(CXX) $(CXX_FLAGS) $^ -o $@ -lpthread -lssl -lcrypto -lprotobuf

%.pb.cc: %.proto
	protoc -I=proto --cpp_out=proto $<

push: cdrone
	scp cdrone drone-home:

run: cdrone
	scp cdrone drone-home:
	ssh drone-home ./cdrone

clean:
	@rm -f cdrone *.o json/jsoncpp.o proto/*.pb.*
