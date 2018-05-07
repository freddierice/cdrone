import logz
import sys
import matplotlib.pyplot as plt
import cv2
import numpy as np
from kalman import kalman


MOTION_REDUCE = 10500 / 2650
def main(log_filename: str):
    """main."""
    with logz.open(log_filename) as l:

        position = l['position']
        vrpn = l['vrpn']

        # plot for the angles
        plt.subplot(311)
        plt.plot(position['time'], position['x'])
        plt.plot(vrpn['time'], vrpn['x'])

        plt.subplot(312)
        plt.plot(position['time'], position['y'])
        plt.plot(vrpn['time'], vrpn['z'])

        plt.subplot(313)
        plt.plot(position['time'], position['z'])
        plt.plot(vrpn['time'], vrpn['y'])
        
        plt.show()

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("usage: %s <run>" % (sys.argv[0],))
        sys.exit(1)
    main(sys.argv[1])
