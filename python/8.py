# -*- coding: utf-8 -*-

from __future__ import print_function

import numpy as np
import scipy as sp
import scipy.linalg
import math


def power_method(a, epsilon):
    def proc():
        np.dot(a, x, out=y)
        v = y.max()
        vs = np.dot(x.T, y) / np.dot(x.T, x)
        np.divide(y, v, out=x)
        return v, vs

    k = 1
    x = np.random.random_sample(len(a)).reshape([-1, 1])
    y = np.ndarray(x.shape)
    v, vs = proc()
    while True:
        k += 1
        vs_prev = vs
        v, vs = proc()
        if abs(vs_prev - vs) < epsilon * abs(vs):
            break
    return vs[0][0], k


def inverse_iteration_method(a, epsilon):
    def proc():
        z[:] = sp.linalg.lu_solve((lu, piv), x)
        w = z.max()
        vs = np.dot(x.T, x) / np.dot(x.T, z)
        np.divide(z, w, out=x)
        return w, vs

    k = 1
    x = np.random.random_sample(len(a)).reshape([-1, 1])
    z = np.ndarray(x.shape)
    lu, piv = sp.linalg.lu_factor(a)
    w, vs = proc()
    while True:
        k += 1
        vs_prev = vs
        w, vs = proc()
        if abs(vs_prev - vs) < epsilon * abs(vs):
            break
    return vs[0][0], k


def main():
    def calculate(method, a, epsilon, true_value):
        fmt = """{method}
  iteration number: {result[1]}
  calculated value: {result[0]}
  true value: {true_value}
  error: {error}"""
        result = globals()['_'.join(method.split(' '))](a, epsilon)
        print(fmt.format(
            method=method,
            result=result,
            true_value=true_value,
            error=abs(true_value - result[0])
        ))

    A = np.ndarray([10, 10])
    for i in xrange(10):
        A[: 10 - i, 9 - i] = i + 1
        A[9 - i, : 10 - i] = i + 1
    calculate('power method', A, 1e-8, 1 / (2 * (1 - math.cos(math.pi / 21))))
    calculate(
        'inverse iteration method',
        A, 1e-8,
        1 / (2 * (1 - math.cos(19 * math.pi / 21)))
    )


if __name__ == '__main__':
    main()
