import struct

class ProtobufIter:
    """Protobuf reader takes in a buffer of bytes, then returns Protobuf objects."""

    def __init__(self, buf, proto_class):
        """initialize the ProtobufIter."""
        self._buf = buf
        self._class = proto_class
        pass

    def __next__(self):
        """return the next protobuf class."""
        if len(self._buf) == 0:
            raise StopIteration()
        ret = self._class()
        proto_len = struct.unpack(">I", self._buf[:4])[0]
        ret.ParseFromString(self._buf[4:4+proto_len])
        self._buf = self._buf[4+proto_len:]
        return ret

    def __iter__(self):
        """Protobuf is the iterator in this case."""
        return self
