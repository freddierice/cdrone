import logz
import sys
import matplotlib.pyplot as plt

HEIGHT_FACTOR = 5000
def main(log_filename: str):
    """main."""
    with logz.open(log_filename) as l:
        rc = l["rc"]
        height = l["height"]
        height_raw = l["height_raw"]
        # imu = l["imu"]
        motion = l["camera_motion"]
        
        # apply filter to gyro
        gyro = l['gyro']
        gyro_x = gyro['x'].copy().astype('i2').astype(float)
        gyro_y = gyro['y'].copy().astype('i2').astype(float)
        gyro_x /= 4.096
        gyro_y /= 4.096
        # gyro_x[gyro_x > 140] -= 280
        # gyro_y[gyro_y > 140] -= 280

        print(l.keys())
        
        has_vrpn = "vrpn" in l.keys()
        if has_vrpn:
            vrpn = l["vrpn"]
        
        # make plot for roll pitch yaw rc commands
        plt.subplot(511)
        plt.plot(rc["time"], rc["throttle"])
        plt.plot(rc["time"], rc["aux1"])
        plt.gca().set_ylim([1350, 1450])

        # make plot for roll pitch yaw
        plt.subplot(512)
        plt.plot(rc["time"], rc["roll"])
        plt.plot(rc["time"], rc["pitch"])
        plt.plot(rc["time"], rc["yaw"])

        # plot for height
        plt.subplot(513)
        plt.plot(height["time"], height["value"])
        plt.plot(height_raw["time"], height_raw["value"])
        if has_vrpn:
            plt.plot(vrpn["time"], vrpn["y"])

        # plot for the angles
        plt.subplot(514)
        # plt.plot(imu["time"], imu["ang_roll"])
        #plt.plot(imu["time"], imu["ang_pitch"])
        plt.plot(gyro["time"], gyro_x)
        # plt.plot(gyro["time"], gyro_y)

        plt.subplot(515)
        # plt.plot(imu["time"][1:], imu["dang_roll"][1:])
        # plt.plot(imu["time"][1:], imu["dang_pitch"][1:])
        # plt.plot(imu["time"], imu["ang_yaw"])
        plt.plot(motion['time'], motion['x'])
        # plt.plot(motion['time'], motion['y'])

        plt.show()

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("usage: %s <run>" % (sys.argv[0],))
        sys.exit(1)
    main(sys.argv[1])
