#!/bin/python3

import numpy as np
from sklearn.impute import KNNImputer
import csv
import sys, os

class Row:
    def __init__(self, time:float = 0, columns:list = []):
        self.time = time
        self.columns = columns

    def __repr__(self):
        return f"{self.time} {self.columns}"
    
    def get_csv(self):
        v = [self.time]
        v.extend(self.columns)
        return ','.join(map(str, v))

class Log:
    def open(self, path):
        with open(path) as file:
            self.reader = csv.reader(file)
            self.parse()
    
    def parse(self):
        rows = self.reader

        self.header = next(rows)

        self.data = []
        self.rows = []

        for row in rows:
            v = []
            for i in range(1, len(row)):
                v.append(float(row[i]))
            r = Row(float(row[0]), v)
            self.rows.append(r)
            self.data.append(v)

    def get_size(self):
        data = self.data
        return len(data), len(self.data[0])

    def get_row_count(self) -> int:
        return len(self.rows)

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
        B[0] = np.nan
        #A = A[1:]
        np.delete(A, row, axis=0)

        combined_data = np.vstack([A, B])

        #print(combined_data)
        print(B)

        imputer = KNNImputer(n_neighbors=3)

        filled_data = imputer.fit_transform(combined_data)

        print(filled_data[-1])

        return filled_data[-1][index]

    def get_row(self, index:int) -> Row:
        return self.rows[index]

    def binary_index(self, time:float, start:int, end:int, depth:int=0) -> int:
        range = end - start

        if range < 2: return start

        mid = start + (range//2)
        middle = self.get_row(mid)

        if middle.time <= time:
            return self.binary_index(time, mid, end, depth+1)
        else:
            return self.binary_index(time, start, mid, depth+1)

    def binary_index(self, time:float) -> int:
        return self.binary_index(time, 0, self.get_row_count())
    
    def binary_search(self, time:float) -> Row:
        return self.get_row(self.binary_index(time))
    
    def get_row_range_indicies(self, index:int, width:int=1):
        size = self.get_row_count()
        if index + width >= size:
            index = size - width - 1
        if index - width < 0:
            index = width

        return [i for i in range(index - width, index + width + 1)]

    def get_rows(self, indicies:list[int]) -> list[Row]:
        return [self.rows[i] for i in indicies]

    def compute_acceleration(self, index:int) -> float:
        rows = self.get_rows(self.get_row_range_indicies(index))
        time_difference = rows[-1].time - rows[0].time
        vel_difference = rows[-1].columns[5] - rows[0].columns[5]
        return vel_difference / time_difference

    def get_csv_header(self):
        return ','.join(self.header)

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

    #print(f"Predicted value: {log.solve_missing(626 - 571, int(sys.argv[2]))}")
    #print('\n'.join(map(str, log.rows)))
    #print(log.get_rows(log.get_row_count(), 2))
    #print(log.compute_acceleration(1659))
    print(log.get_csv_header())
    for i in range(log.get_row_count()):
        row = log.get_row(i)
        row.columns.append(log.compute_acceleration(i))
        print(row.get_csv())