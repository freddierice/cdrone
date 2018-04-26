import cv2
import numpy as np

def kalman(x, process, measurement):
    kf = cv2.KalmanFilter(1, 1)
    kf.measurementMatrix = np.array([1], np.float32)
    kf.transitionMatrix = np.array([1], np.float32)
    kf.processNoiseCov = np.array([process], np.float32)
    kf.measurementNoiseCov = np.array([measurement], np.float32)
    mp = np.ndarray((1,), np.float32)
    cp = np.ndarray((1,), np.float32)
    new_x = np.empty_like(x)
    for i in range(len(x)):
        mp[0] = x[i]
        kf.correct(mp)
        new_x[i] = kf.predict()
    return new_x
