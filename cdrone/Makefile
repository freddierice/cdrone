PREFIX:=armv8-rpi3-linux-gnueabihf-
CXX:=$(PREFIX)g++
CXX_FLAGS:=-Wall -std=c++14 -O2 -I/usr/local/include # -g -Og # -O2
LIBS:=-lpthread -lssl -lcrypto -lprotobuf -lopencv_core -lopencv_video -lopencv_imgcodecs -larchive -Wl,-rpath=/opt/vc/lib -L/opt/vc/lib -lbcm_host -lmmal -lmmal_core -lmmal_components -lmmal_util -lvrpn -lquat -lwiringPi
TARGET:=cdrone

.PHONY: docker pkg

PROTOS=$(patsubst ../proto/%.proto,proto/%.pb.h,$(wildcard ../proto/*.proto))
all: logging/generator/generator logging/generated.cpp proto $(PROTOS) $(TARGET)

#MODULES=controller hardware json programs wire
OBJS=$(patsubst %.cpp,%.o,$(wildcard */*.cpp)) $(patsubst %.cc,%.o,$(wildcard */*.cc)) main.o proto/io.pb.o proto/log.pb.o logging/generated.o

%.cpp: %.h
%.cc: %.h

logging/generator/generator:
	@make -C logging/generator

logging/generated.cpp:
	@./logging/generator/generator ./logging/generator/generator.conf ./logging

proto/%.pb.cc proto/%.pb.h: ../proto/%.proto
	protoc -I=../proto --cpp_out=proto $<

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -I. -I/opt/vc/include -c $^ -o $@

%.o: %.cc
	$(CXX) $(CXX_FLAGS) -I. -c $^ -o $@

%.o: %.c
	$(CXX) $(CXX_FLAGS) -I. -c $^ -o $@

proto:
	@mkdir proto

docker:
	@docker run -it --rm -v $(shell pwd):/home/user/cdrone -v $(shell pwd)/../proto:/home/user/proto freddierice/cdrone /usr/bin/make -j4

$(TARGET): $(OBJS)
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(LIBS)

clean:
	@rm -f $(TARGET) *.o */*.o proto/*.pb.* logging/generator/generator logging/generated.cpp

pkg:
	./pkg
