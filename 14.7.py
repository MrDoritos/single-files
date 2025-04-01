def print_all_permutations(permList, nameList):
    if not len(nameList):return print(*permList,sep=', ')
    for x in(X:=nameList):i=X.index(x);print_all_permutations(permList+[x],X[:i]+X[i+1:])

if __name__ == "__main__": 
    nameList = input().split(' ')
    permList = []
    print_all_permutations(permList, nameList)