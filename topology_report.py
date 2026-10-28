#!/bin/python3
"""
\( *(\d+), *(\d+)\)

\d+ \| (?:([\+\-0-9.]+) +)+

(?!\d+ +\| +)(?:(?P<DOT>\.).*)+

"""
import sys, os

def error_exit(ws:str, ec:int=1):
    print(ws)
    exit(ec)

if len(sys.argv) < 2:
    error_exit("Not enough args")

filename = sys.argv[1]

if not os.path.exists(filename):
    error_exit("File does not exist")

file = None

try:
    file = open(filename, 'r')
except:
    error_exit("Failed to open file")

line = None

while (len(line := file.readline())):
    if line.find("Bed Topography Report") != -1:
