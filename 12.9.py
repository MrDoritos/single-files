u,C,f=open(input()).readlines(),[[],[],[]],open('report.txt','w')
for i in u:
    i,t=i.strip(),'\t';g=[int(x) for x in i.split(t)[2:]];a=sum(g)/len(g);f.write(i+t+['A',['B',['C',['D','F'][a<60]][a<70]][a<80]][a<90]+'\n')
    for I,P in zip(g,C):P+=[I]
f.write(f'\nAverages: midterm1 {sum(C[0])/len(C[0]):.2f}, midterm2 {sum(C[1])/len(C[1]):.2f}, final {sum(C[2])/len(C[2]):.2f}\n')