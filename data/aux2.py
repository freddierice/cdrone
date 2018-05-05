import logz
import sys
import matplotlib.pyplot as plt

def main(log_filename: str):
    """calculate frequencies of different measurements."""
    with logz.open(log_filename) as l:
        print(l['rc']['aux2'])

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("usage: %s <run>" % (sys.argv[0],))
        sys.exit(1)
    main(sys.argv[1])
