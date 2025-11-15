#!/bin/python3
import math

principal=25000
double_rate=math.pow(2,0.2)
age_base=22
age_max=65
monthly_base=100
monthly_increase=100
monthly_max=1000

def monthly_rate(monthly, rate, terms):
    annual = monthly * 12
    mult = math.pow(rate, terms)
    return annual * mult

def principal_rate(principal, rate, terms):
    return principal * math.pow(rate, terms)

def print_growth():
    monthly_payments=[f"**{i}**" for i in range(monthly_base, monthly_max+1, monthly_increase)]
    print("**Year**,**Age**,**Principal**", *monthly_payments, sep=',')
    
    for i in range(0, age_max-age_base+1):
        print(f"{i},{i+age_base},{principal_rate(principal, double_rate, i):.0f}", *[f"{monthly_rate(monthly, double_rate, i):.0f}" for monthly in range(monthly_base, monthly_max+1, monthly_increase)], sep=',')

print_growth()
