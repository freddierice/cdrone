import logz
import sys
import matplotlib.pyplot as plt

def main(log_filename: str):
    """calculate frequencies of different measurements."""
    with logz.open(log_filename) as l:
        print("frequencies in Hz: ")
        for x in l.keys():
            var = l[x]
            if len(var) > 0:
                freq = len(var) / (var['time'][-1] - var['time'][0]) * 1000000
            else:
                freq = float(0)
            print("%s\t%f" % (x, freq))

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("usage: %s <run>" % (sys.argv[0],))
        sys.exit(1)
    main(sys.argv[1])
