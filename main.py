import time

import matplotlib.pyplot as plt
import numpy as np

from hathaani import Hathaani
import librosa
from pitchTrack import PitchTrack
from utils import Util
import sys
import signal
from definitions import *

np.set_printoptions(precision=4, threshold=sys.maxsize, suppress=True)

robot = Hathaani(tonic='D3')    # TONIC(D3) = 50


def handle_signal(sig, frame):
    robot.terminate()


if __name__ == '__main__':
    signal.signal(signal.SIGINT, handler=handle_signal)

    robot.home()
    while not robot.ready:
        time.sleep(0.25)

    audio, fs = librosa.load("vocal_16k.wav", sr=16000, duration=120)
    hop_size = int(fs // 100)

    bounds = Util.get_silence_bounds(audio, fs)
    temp = Util.split(audio, bounds, int(fs / 2))
    a = temp[1][0]

    pitch_tracker = PitchTrack('pyin', hop_size=hop_size)
    e = Util.envelope(a, hop_size)
    cents = pitch_tracker.track(a, fs, return_cents=True, fill_gaps=False)
    cents, e = Util.truncate_phrase(cents, e)
    n = min(len(cents), len(e))
    phrase = np.vstack([cents, e])
    robot.perform(phrase, phrase_tonic=48, interpolate_start=True, interpolate_end=True)

    # fs = 16000
    # hop = 160
    # tp = hop / fs
    # t = 0.25  # s
    # N = int(t / tp)
    # print(f"Num points: {N}")
    #
    # p_traj = Util.interp(BOW_HEIGHT.REST, BOW_HEIGHT.E.MIN, N)
    # p_traj = np.hstack([p_traj, np.zeros(7*N) + BOW_HEIGHT.E.MIN])
    # p_traj = np.hstack([p_traj, Util.interp(BOW_HEIGHT.E.MIN, BOW_HEIGHT.E.MAX, N)])
    # p_traj = np.hstack([p_traj, np.zeros(8 * N) + BOW_HEIGHT.E.MAX])
    # p_traj = np.hstack([p_traj, Util.interp(BOW_HEIGHT.E.MAX, BOW_HEIGHT.REST, N)]).reshape(-1, 1)
    #
    # a_traj = Util.interp(0, BOW_ANGLE.E, N)
    # a_traj = np.hstack([a_traj, np.zeros(16*N) + BOW_ANGLE.E])
    # a_traj = np.hstack([a_traj, Util.interp(BOW_ANGLE.E, BOW_ANGLE.REST, N)]).reshape(-1, 1)
    #
    # r_traj = Util.interp(BOW_ROTOR.REST, BOW_ROTOR.MAX, 9*N, tb_percentage=0.01)
    # r_traj = np.hstack([r_traj, Util.interp(BOW_ROTOR.MAX, BOW_ROTOR.REST, 9*N, tb_percentage=0.01)]).reshape(-1, 1)
    # traj = np.hstack([p_traj, a_traj, r_traj])
    # print(traj.shape)
    # robot.perform_trajectory(traj, motor_ids=[MotorId.BOW_D_LEFT, MotorId.BOW_D_RIGHT, MotorId.BOW_ROTOR])