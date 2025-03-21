#!/bin/python3

class StudentInfoError(Exception):
    def __init__(self, message):
        self.message = message

def find_ID(name, info):
    if name not in info:
        raise StudentInfoError(f"Student ID not found for {name}")
    return info[name]
    
def find_name(ID, info:dict):
    if ID not in info.values():
        raise StudentInfoError(f"Student name not found for {ID}")
    #return [x for x in info.keys() if info[x] == ID][0]
    #return [x for x in info if info[x] == ID][0]
    return [x for x in info if info[x]==ID][0]
    #return [*info][[*info.values()].index(ID)]

if __name__ == '__main__':
    student_info = {
        'Reagan' : 'rebradshaw835',
        'Ryley' : 'rbarber894',
        'Peyton' : 'pstott885',
        'Tyrese' : 'tmayo945',
        'Caius' : 'ccharlton329'
    }
    
    userChoice = input()
    
    #try:
    if userChoice == "0":
        name = input()
        result = find_ID(name, student_info)
    else:
        ID = input()
        result = find_name(ID, student_info)

    print(result)
    #except Exception as e:
        #print(e.message)