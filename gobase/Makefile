TARGET:=gobase

PROTOS=$(patsubst ../proto/%.proto,proto/%.pb.go,$(wildcard ../proto/*.proto))
all: $(TARGET)

proto/%.pb.go: ../proto/%.proto
	protoc -I=../proto --go_out=proto ../proto/*.proto

docker:
	@docker run -it --rm -v $(shell pwd):/home/user/cdrone -v $(shell pwd)/../proto:/home/user/proto freddierice/cdrone

$(TARGET): $(PROTOS)
	go build -o $@

clean:
	@rm -f $(TARGET) proto/*.pb.go
