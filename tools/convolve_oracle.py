import struct
import numpy as np
from scipy.signal import convolve2d


def unpack(path):
    with open(path, "rb") as f:
        rows = struct.unpack("i", f.read(4))[0]
        cols = struct.unpack("i", f.read(4))[0]
        data = struct.unpack("I" * (rows * cols), f.read(4 * rows * cols))
        return np.array(data,dtype="I").reshape((rows, cols))


def convolve(a_path,b_path,out_path):
    a = unpack(a_path)
    b = unpack(b_path)
    out = convolve2d(a, b, mode="valid")

    with open(out_path, "wb") as f:
        f.write(struct.pack("i", out.shape[0]))
        f.write(struct.pack("i", out.shape[1]))
        f.write(struct.pack("I" * (out.shape[0] * out.shape[1]), *out.flatten()))

# convolve("/home/flwfdd/code/tmp/cs61c-proj4/tests/test_2d_small/task0/a.bin",
#          "/home/flwfdd/code/tmp/cs61c-proj4/tests/test_2d_small/task0/b.bin",
#          "/home/flwfdd/code/tmp/cs61c-proj4/tests/test_2d_small/task0/out.bin")
