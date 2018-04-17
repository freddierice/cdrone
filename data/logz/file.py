import numpy as np
import tarfile

from . import log_pb2
from .proto import ProtobufIter
from .np_types import c_map

class MapEntry():
    """MapEntry is a entry into the map file."""

    def __init__(self, name: str, dtype, idx: int):
        """initialize a MapEntry."""
        self.name = name
        self.dtype = dtype
        self.idx = idx
        pass


class File():
    """File holds a reference to a logz file."""

    def __init__(self, name: str) -> None:
        """initialize a file with a name."""
        self._file = tarfile.open(name, mode='r')
        try:
            map_info = self._file.getmember("map")
        except KeyError as e:
            raise RuntimeError("bad file: map does not exist")
        with self._file.extractfile(map_info) as map_file:
            map_bytes = map_file.read()
        proto_iter = ProtobufIter(map_bytes, log_pb2.Map)

        self._mappings = {}
        for p in proto_iter:
            if len(p.type_names) != len(p.var_names):
                raise RuntimeError("bad map: type_names needs to be same length as var_names")
            dtype = [(x[0], c_map[x[1]])  for x in zip(p.var_names, p.type_names)]
            # prepend the time
            dtype =  [("time", c_map["uint64_t"])] + dtype
            self._mappings[p.name] = MapEntry(p.name, dtype, p.id)

    def __getitem__(self, key) -> np.ndarray:
        """read a variable from the log"""
        if key not in self._mappings:
            raise KeyError("value '%s' does not exist" % (key,))

        map_entry = self._mappings[key]
        try:
            data_info = self._file.getmember(str(map_entry.idx))
        except KeyError as e:
            raise RuntimeError("bad file: map does not exist")
        with self._file.extractfile(data_info) as data_file:
            data_bytes = data_file.read()
            return np.frombuffer(data_bytes, dtype=map_entry.dtype)

    def closed(self) -> bool:
        """check if the file is closed."""
        return self._file is None

    def close(self) -> None:
        """close the file."""
        self._file.close()
        self._file = None

    def _check(self) -> None:
        if self.closed():
            raise OSError("%s is closed" % self.__class__.__name__)

    def __enter__(self):
        """enter will open the logz file."""
        return self

    def __exit__(self, _type, value, traceback) -> None:
        """exit will close the logz file."""
        self._file.__exit__(_type, value, traceback)
        self.close()

def open(name: str) -> File:
    """open a new file given a name."""
    return File(name)

