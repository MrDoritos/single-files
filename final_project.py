#!/bin/python3
"""
1 2 2 2 3 3 4 5 6

the output is:

Minimum: 1
Maximum: 6
Mean: 3.1
Palindrome: False
Median: 3.0
Mode: 2
"""

#l=[int(x)for x in open().split()if(c:=c+1)]

#l=list(map(int,input().split()));c,o,M,m=len(l),sorted(l),max,min
#print(f"\
#Minimum: {min(l)}\n\
#Maximum: {max(l)}\n\
#Mean: {sum(l)/c:.1f}\n\
#Palindrome: {sum(l[i]==l[c-i-1]for i in range(c))>c//2}\n\
#Median: {(o[c//2]+o[(c-1)//2])/2:.1f}\n\
#Mode: {max((l.count(i),i)for i in l)[1]}")

l=list(map(int,input().split()));c,o,M,m,s=len(l),sorted(l),max,min,sum;print(f"Minimum: {m(l)}\nMaximum: {M(l)}\nMean: {s(l)/c:.1f}\nPalindrome: {s(l[i]==l[-1-i]for i in range(c))>c//2}\nMedian: {(o[c//2]+o[(c-1)//2])/2:.1f}\nMode: {M((l.count(i),i)for i in l)[1]}")
