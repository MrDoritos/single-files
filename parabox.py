import sys,os,math

parabox2 = [
    [6144, 24576, 20],
    [12165, 48660, 22],
    [24330, 97320, 24],
    [36495, 145980, 26],
    [48660, 194640, 28],
    [60825, 243300, 30],
    [72990, 291960, 32],
    [85155, 340620, 34],
    [97320, 389280, 36],
    [109485, 437940, 38],
    [121650, 486600, 40],
]

def min_to_tick(minutes):
    return minutes * 60 * 20

def total_rf(rf, minutes):
    return rf * min_to_tick(minutes)

sum_rf = 0
print("Iteration,Max RF/T,Iteration RF,Total RF")
for i in range(len(parabox2)):
    j = parabox2[i]
    rf_tick = j[1]
    iter_rf = total_rf(j[1], j[2])
    sum_rf += iter_rf
    print(f"{i},{rf_tick},{iter_rf},{sum_rf}")
