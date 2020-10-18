import sys
import random 

MAX_INT = 2147483647*4

list_of_accounts = []

def doStuff(accounts, operations):

    accounts = int(accounts)
    operations = int(operations)
    for x in range(0, accounts):
        list_of_accounts.append(x)

    with open("./InitialBankAccounts.in", "w") as f:
        f.write(str(accounts) + "\n")
        for x in range(0, accounts):
            f.write(str(list_of_accounts[x]) + " " + str(random.randint(0, MAX_INT)) + "\n")
    
    with open("./SomeTransfers.in", "w") as f:
        f.write(str(operations) + "\n")
        for x in range(0, operations):
            f.write(str(list_of_accounts[random.randint(0,len(list_of_accounts)-1)]) + " " + \
                    str(list_of_accounts[random.randint(0,len(list_of_accounts)-1)]) + " " + \
                    str(random.randint(0, MAX_INT/2)) + "\n") 
        
    

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Bad input!\n\tusage: <number_of_accounts> <number_of_operations>")
    print(sys.argv[1], sys.argv[2])
    doStuff(sys.argv[1], sys.argv[2])
