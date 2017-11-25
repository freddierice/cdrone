PREFIX:=armv8-rpi3-linux-gnueabihf-
CXX:=$(PREFIX)g++
CXX_FLAGS:=-Wall -std=c++14 -DSPDLOG_DEBUG_ON -Og -g # -O2
LIBS:=-lpthread -lssl -lcrypto -lprotobuf -latomic -lbcm_host -lmmal -lmmal_core # -lmmal_components
TARGET:=cdrone

PROTOS=$(patsubst ../proto/%.proto,proto/%.pb.h,$(wildcard ../proto/*.proto))
all: $(PROTOS) $(TARGET)

#MODULES=controller hardware json programs wire
OBJS=$(patsubst %.cpp,%.o,$(wildcard */*.cpp)) $(patsubst %.cc,%.o,$(wildcard */*.cc)) main.o proto/io.pb.o

# TODO: do expansion for cpp and cc files that actually have .h files.
%.cpp: %.h
%.cc: %.h

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -I. -c $^ -o $@

%.o: %.cc
	$(CXX) $(CXX_FLAGS) -I. -c $^ -o $@

proto/%.pb.cc proto/%.pb.h: ../proto/%.proto
	protoc -I=../proto --cpp_out=proto $<

$(TARGET): $(OBJS)
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(LIBS)

# phony directives
push: $(TARGET)
	scp $(TARGET) drone-arch:

run: $(TARGET)
	scp $(TARGET) drone-arch:
	ssh drone-home ./$(TARGET)

clean:
	@rm -f $(TARGET) *.o */*.o proto/*.pb.*
