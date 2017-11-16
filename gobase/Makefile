TARGET:=gobase

PROTOS=$(patsubst ../proto/%.proto,proto/%.pb.go,$(wildcard ../proto/*.proto))
all: $(TARGET)

proto/%.pb.go: ../proto/%.proto
	protoc -I=../proto --go_out=proto $<

$(TARGET): $(PROTOS)
	go build -o $@

clean:
	@rm -f $(TARGET) proto/*.pb.go
