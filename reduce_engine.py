#!/bin/python3

import numpy as np
from sklearn.impute import KNNImputer
from scipy.fft import fft, fftfreq
import matplotlib.pyplot as plt
import csv
import sys, os, copy

class Util:
    def lerp(v1, v2, factor):
        return v1 * (1 - factor) + v2 * factor

class Row:
    def __init__(self, time:float = None, columns:list = None):
        self.time = time if time is not None else time
        self.columns = columns if columns is not None else []

    def __repr__(self):
        return f"{self.time} {self.columns}"
    
    def get_csv(self):
        v = [self.time]
        v.extend(self.columns)
        return ','.join(map(str, v))
    
    def get_factor(self, row, time:float) -> float:
        return (time - self.time) / (row.time - self.time)

    def interpolate(self, other, factor:float):
        ret = Row(0, [])

        ret.time = Util.lerp(self.time, other.time, factor)

        for i in range(len(self.columns)):
            ret.columns.append(
                Util.lerp(
                    self.columns[i], 
                    other.columns[i], 
                    factor
                )
            )

        return ret

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

    def binary_index_r(self, time:float, start:int, end:int, depth:int=0) -> int:
        range = end - start

        if range < 2: return start

        mid = start + (range//2)
        middle = self.get_row(mid)

        if middle.time <= time:
            return self.binary_index_r(time, mid, end, depth+1)
        else:
            return self.binary_index_r(time, start, mid, depth+1)

    def binary_index(self, time:float) -> int:
        return self.binary_index_r(time, 0, self.get_row_count())
    
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

    def time_pair_indicies(self, time:float):
        i = self.binary_index(time)

        if i < 1:
            if self.get_row_count() < 1:
                return None, None
            return 0, 1
        
        if i >= self.get_row_count() - 1:
            size = self.get_row_count()
            return size - 2, size - 1
        
        return i, i+1
    
    def time_pair(self, time:float):
        a, b = self.time_pair_indicies(time)

        if not a or not b:
            return None, None
        
        return self.get_row(a), self.get_row(b)
    
    def interpolate_row(self, time:float):
        v1, v2 = self.time_pair(time)

        if not v1 or not v2:
            return None

        factor = v1.get_factor(v2, time)

        return v1.interpolate(v2, factor)
    
    def get_time_minmax(self):
        min = max = self.get_row(0).time

        for row in self.rows:
            if min > row.time:
                min = row.time
            if max < row.time:
                max = row.time

        return min, max
    
    def get_time_range(self) -> float:
        min, max = self.get_time_minmax()
        return max - min

    def compute_acceleration(self, index:int) -> float:
        rows = self.get_rows(self.get_row_range_indicies(index))
        time_difference = rows[-1].time - rows[0].time
        vel_difference = rows[-1].columns[5] - rows[0].columns[5]
        return vel_difference / time_difference

    def compute_acceleration_interpolated(self, time:float, time_width:float=0.1, index:int=5) -> float:
        a = self.interpolate_row(time - time_width)
        b = self.interpolate_row(time + time_width)

        if not a or not b:
            return 0

        return (b.columns[index] - a.columns[index]) / (b.time - a.time)

    def get_csv_header(self):
        return ','.join(self.header)

    def __repr__(self):
        ret = str(self.header) + "\n"
        for row in self.data:
            ret += str(row) + "\n"
        return ret
    
    def plot_hist(self, sample_rate_hz:float):
        min, max = log.get_time_minmax()

        time = min
        step = 1 / sample_rate_hz
        samples = []

        while (time < max):
            #samples.append(self.compute_acceleration_interpolated(time, step, 0))
            print(f'\r{time}', end='')
            row = self.interpolate_row(time)
            time += step
            if (not row) or (row.columns[0] < 0.1):
                continue
            ratio = row.columns[5] / row.columns[0]

            if ratio < 0.0001:
                continue
            samples.append(row.columns[5] / row.columns[0])

        #N = len(samples)

        #yf = fft(samples)
        #xf = fftfreq(N, step)

        plt.hist(samples, bins=500, color='skyblue')
        plt.grid()
        plt.xlabel('Speed / RPM')
        plt.ylabel('Count')
        plt.title('Histogram')
        plt.show()

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

    #for i in range(log.get_row_count()):
    #    row = log.get_row(i)
    #    row.columns.append(log.compute_acceleration(i))
    #    print(row.get_csv())

    
    #min, max = log.get_time_minmax()
    #time = min
    #step = 0.1
    #while (time < max):
    #    row = log.interpolate_row(time)
    #    if row:
    #        #print(f"\r{time} / {max}", file=sys.stderr, end='')
    #        #row2 = copy.deepcopy(row)
    #        row.columns.append(log.compute_acceleration_interpolated(time, step))
    #        print(row.get_csv())
    #    time += step

    log.plot_hist(100)