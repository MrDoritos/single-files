#!/bin/python3

def fibonacci(n):
    a,b,c=0,1,(n==1)*1
    return -1 if n<0 else [0,[c for _ in range(1,n) if (c:=a+b) and (a:=b) and (b:=c) or 1]][-1]
    #if (n < 0): return -1
    #a,b,c=0,1,(n==1)*1
    #for _ in range(1,n):c,a=a+b,b;b=c
    #return c

def fibonacci(n):
    a,b,c=0,1,(n==1)*1
    return -1 if n<0 else max([0]+[c for _ in range(1,n) if (c:=a+b) and (a:=b) and (b:=c) or 1])

print([fibonacci(x) for x in range(-1,10)])

def fibonacci(n):a,b,c=0,1,(n==1)*1;return -1 if n<0 else max([c]+[c for _ in range(1,n)if(c:=a+b)&(a:=b)&(b:=c)|1])

print([fibonacci(x) for x in range(-1,10)])
