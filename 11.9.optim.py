from datetime import*
D=date;S,P,N,Y=D.strftime,print,"\n","%Y"
#for x in(v:=sorted([D.fromisoformat(input())\
#for _ in"    "])):P(S(x,"%m/%d/%Y")) \
#for _ in N*4])):P(S(x,"%m/%d/%Y"))
#a,b,c,d=v=sorted(map(D.fromisoformat,open(0).read().splitlines()))
#print(map(S("%m/%d/%Y"),v))
#for x in v:P(S(x,'%m/%d/%Y'))

#a,b,c,d=sorted([D.fromisoformat(input())for _ in N*4])
a,b,c,d=[x for x in sorted(map(D.fromisoformat,map(input,'\0'*4)))if P(S(x,"%m/%d/%Y"))]

#P((v[3]-v[2]).days);P(S(timedelta(21)+v[3],"%B %d, %Y"));P(S(v[0],"%A"))

#a,b,c,d=v;print(a,b,c,d,sep='\n')

#P((d-c).days);P(S(timedelta(21)+d,"%B %d, %Y"));P(S(a,"%A"))

P((d-c).days,S(timedelta(21)+d,"%B %d, %Y"),S(a,"%A"),sep=N)
