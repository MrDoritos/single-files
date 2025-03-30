from datetime import*
D=date;P,S=print,D.strftime
def read_date():return D.fromisoformat(input())
for x in(v:=sorted([read_date()for x in"    "])):P(S(x,"%m/%d/%Y"))
P((v[3]-v[2]).days);P(S(timedelta(21)+v[3],"%B %d, %Y"));P(S(v[0],"%A"))