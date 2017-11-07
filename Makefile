PREFIX:=armv8-rpi3-linux-gnueabihf-
CXX:=$(PREFIX)g++
CXX_FLAGS:=-Wall -std=c++14 -O2
LIBS:=-lpthread -lssl -lcrypto -lprotobuf
TARGET:=cdrone

all: $(TARGET)

#MODULES=controller hardware json programs wire
OBJS=$(patsubst %.cpp,%.o,$(wildcard */*.cpp)) $(patsubst %.cc,%.o,$(wildcard */*.cc)) main.o proto/io.pb.o

# TODO: do expansion for cpp and cc files that actually have .h files.
%.cpp: %.h
%.cc: %.h

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -I. -c $^ -o $@

%.o: %.cc
	$(CXX) $(CXX_FLAGS) -I. -c $^ -o $@

%.pb.cc: %.proto
	protoc -I=proto --cpp_out=proto $<

$(TARGET): $(OBJS)
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(LIBS)

# phony directives
push: $(TARGET)
	scp $(TARGET) drone-home:

run: $(TARGET)
	scp $(TARGET) drone-home:
	ssh drone-home ./$(TARGET)

clean:
	@rm -f $(TARGET) **/*.o proto/*.pb.*
