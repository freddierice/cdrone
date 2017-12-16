PREFIX:=armv8-rpi3-linux-gnueabihf-
CXX:=$(PREFIX)g++
CXX_FLAGS:=-Wall -std=c++14 -O2 #-g -Og -O2
LIBS:=-lpthread -lssl -lcrypto -lprotobuf -lbcm_host -lmmal -lmmal_core -lmmal_components -lmmal_util -lopencv_core -lopencv_video -lopencv_imgcodecs # -latomic 
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

clean:
	@rm -f $(TARGET) *.o */*.o proto/*.pb.*
