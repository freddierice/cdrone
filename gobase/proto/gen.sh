#!/bin/bash
## generate the protobuf source files
protoc -I../../proto/ --go_out=. ../../proto/*.proto
