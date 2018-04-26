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
        motion = l["camera_motion"].copy()
        motion_kf = l["camera_motion"].copy()

        motion_kf['x'][:] = kalman(motion['x'], 0.0001, 0.1)[:]

        vrpn = l['vrpn']
        vrpn['x']
        vrpn_dx = logz.dt(vrpn, 'x')
        vrpn_dz = logz.dt(vrpn, 'z')

        # plot for the angles
        plt.subplot(411)
        plt.plot(vrpn['time'][:-1], vrpn_dx)

        plt.subplot(412)
        plt.plot(vrpn['time'][:-1], vrpn_dz)
        
        plt.subplot(413)
        plt.plot(motion['time'], motion['x'])

        plt.subplot(414)
        plt.plot(motion['time'], motion['y'])

        plt.show()

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("usage: %s <run>" % (sys.argv[0],))
        sys.exit(1)
    main(sys.argv[1])
