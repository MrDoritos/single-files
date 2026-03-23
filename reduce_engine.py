#!/bin/python3

import numpy as np
from sklearn.impute import KNNImputer
import csv
import sys, os

class Log:
    def open(self, path):
        with open(path) as file:
            self.reader = csv.reader(file)
            self.parse()
    
    def parse(self):
        rows = self.reader

        self.header = next(rows)

        self.data = []

        for row in rows:
            v = []
            for i in range(1, len(row)):
                v.append(float(row[i]))
            self.data.append(v)

    def get_size(self):
        data = self.data
        return len(data), len(self.data[0])

    def get_train(self):
        data = self.data
        return np.array(self.data), np.array(self.data[0])

    def reduce(self):
        M = len(self.data)
        N = len(self.data[0])

        A = np.array(self.data)
        B = np.array(self.data[0])
        #A = np.random.rand(M, N)
        #print(A)
        B = np.random.rand(M)
        x, residuals, rank, s = np.linalg.lstsq(A, B, rcond=None)

        print(f"Solved coefficients shape: {x.shape}") # Should be (12,)
        print(f"Sum of squared residuals: {residuals}")
        print(f"s {s}")
        print(f"Rank {rank}")

    def solve_missing(self, row, index):
        M, N = self.get_size()
        A, B = self.get_train()

        B=A[row]

        print(B)

        B[index] = np.nan
        #A = A[1:]
        np.delete(A, row, axis=0)

        combined_data = np.vstack([A, B])

        #print(combined_data)
        print(B)

        imputer = KNNImputer(n_neighbors=3)

        filled_data = imputer.fit_transform(combined_data)

        return filled_data[-1][index]


    def __repr__(self):
        ret = str(self.header) + "\n"
        for row in self.data:
            ret += str(row) + "\n"
        return ret

if __name__ == "__main__":
    np.set_printoptions(suppress=True)
    log = Log()
    if len(sys.argv) < 3:
        print("Not enough args")
        exit(1)
    log.open(sys.argv[1])
    #print(log)
    #log.reduce()

    print(f"Predicted value: {log.solve_missing(626 - 571, int(sys.argv[2]))}")