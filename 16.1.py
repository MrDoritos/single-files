from datetime import datetime,date
import re

while(i:=input())!='-1':print(end=datetime.strptime(i,"%B %d, %Y").strftime("%-m/%-d/%Y\n")if re.match("\w+ \d+, \d+",i)else'')
