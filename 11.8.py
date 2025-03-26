import random as r
def number_guess(num):n=r.randint(1,100);print(num,"is",[f"too {['low','high'][num>n]}. Random number was {n}.","correct!"][num==n])
if __name__=="__main__":r.seed(900);_=[number_guess(int(x))for x in input().split()]