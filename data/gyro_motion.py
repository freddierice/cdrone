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
        height = l["height"]
        height_raw = l["height_raw"]

        motion = l["camera_motion"].copy()
        motion_kf = l["camera_motion"].copy()
        start = motion_time = motion['time'][0]
        end = motion_time = motion['time'][-1]

        print("seconds elapsed: %f" % ((end - start)/1000000,))
        
        gyro = l['gyro'].copy()
        gyro_kf = l['gyro'].copy()

        #(new_time, new_motion, new_gyro) = logz.bin_vars(motion, gyro)
        # new_motion_x = new_motion['x'] + new_gyro['x']
        gyro_kf['x'][:] = kalman(gyro['x'], 0.1, 10)[:]
        motion_kf['x'][:] = kalman(motion['x'], 0.0001, 0.01)[:]

        (gyro_bin, motion_bin) = logz.bin_up(gyro_kf, motion_kf)

        # (new_time, new_gyro, new_motion) = logz.bin_vars(gyro_kf, motion_kf)
        # fit = np.polyfit(gyro_bin['x'], motion_bin['x'], 1)
        motion_bin['x'] = motion_bin['x'] - (gyro_bin['x']*-1.0)
        # print(fit)

        motion_done = motion_bin.copy()
        motion_done['x'][:] = kalman(motion_bin['x'], 0.00001, 0.001)
        
        # plot for height
        # plt.subplot(411)
        # plt.plot(height["time"], height["value"])
        # plt.plot(height_raw["time"], height_raw["value"])

        # plot for the angles
        plt.subplot(611)
        # plt.plot(new_time, new_gyro['x'])
        plt.plot(gyro["time"], gyro['x'])

        plt.subplot(612)
        plt.plot(gyro_kf['time'], gyro_kf['x'])
        # plt.plot(new_time, new_motion['x'])
        # plt.plot(motion['time'], motion['y'])

        plt.subplot(613)
        plt.plot(motion['time'], motion['x'])

        plt.subplot(614)
        plt.plot(motion_kf['time'], motion_kf['x'])

        plt.subplot(615)
        plt.plot(motion_bin['time'], motion_bin['x'])

        plt.subplot(616)
        plt.plot(motion_done['time'], motion_done['x'])

        plt.show()

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("usage: %s <run>" % (sys.argv[0],))
        sys.exit(1)
    main(sys.argv[1])
