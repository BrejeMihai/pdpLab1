#include "utils.h"
#include <malloc.h>
#include <windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <string.h>
#include <fstream>
#include <iostream>

#include "list.h"

NODE*
CreateAList()

{
    NODE* head = NULL;
    head = allocateNode();

    if (NULL == head)
    {
        return NULL;
    }

    head->Info.accountTransferA = NULL;
    head->Info.accountTransferB = NULL;
    head->Info.amount = NULL;

    InitializeListHead(&(head->ListEntry));

    return head;
}


NODE*
allocateNode()

{
    NODE* newNode = NULL;

    newNode = (NODE*)malloc(sizeof(NODE));
    if (NULL == newNode)
    {
        return NULL;
    }
    newNode->Info.accountTransferA = NULL;
    newNode->Info.accountTransferB = NULL;
    newNode->Info.amount = NULL;

    return newNode;
}


void 
insertInListEntry(
    NODE * ListEntryHead, 
    unsigned __int64 AccountA, 
    unsigned __int64 AccountB, 
    unsigned __int64 Amount, 
    CRITICAL_SECTION* CriticalSectionLock, 
    HANDLE EventGoOn
)

{
    NODE* newNode = NULL;
    newNode = allocateNode();
    if (NULL == newNode)
    {
        return;
    }

    newNode->Info.accountTransferA = AccountA;
    newNode->Info.accountTransferB = AccountB;
    newNode->Info.amount = Amount;

    EnterCriticalSection(CriticalSectionLock);
    InterlockedInsertTailList(&ListEntryHead->ListEntry, &newNode->ListEntry, CriticalSectionLock);
    LeaveCriticalSection(CriticalSectionLock);

    SetEvent(EventGoOn);
    return;
}


DWORD 
WINAPI 
SetupInitialAccounts(
    LPVOID lpParam
)

{
    int numberOfAccounts = 0;
    unsigned __int64 accountNumber    = 0;
    unsigned __int64 moneyAmount      = 0;

    PTHREAD_PARAMETERS pthreadParameters = (PTHREAD_PARAMETERS)lpParam;

    std::ifstream infile("D:\\UBB-932\\Sem1\\PDP\\Lab01\\BankAccounts\\BankAccounts\\InitialBankAccounts.in");
    

    if (!(infile >> numberOfAccounts))
    {
        fprintf_s(stderr, "Couldn't read initial bank accounts number\n");
        return (DWORD)-1;
    }
    if (0 > numberOfAccounts)
    {
        fprintf_s(stderr, "Negative number of accounts, bad!\n");
        return (DWORD)-1;
    }
    

    while (numberOfAccounts--)
    {
        if (!(infile >> accountNumber >> moneyAmount))
        {
            continue;
        }
        PBANK_ACCOUNT newAccount = CreateBankAccount(accountNumber, moneyAmount);
        pthreadParameters->BankAccounts.emplace(newAccount->accountIdentifier, newAccount);
    }

    fprintf_s(stdout, "All bank accounts were initialized!\n");
    return 0;
}


DWORD
WINAPI 
ParseOperations(
    LPVOID lpParam
)

{
    PTHREAD_PARAMETERS pthreadParameters = (PTHREAD_PARAMETERS)lpParam;

    std::ifstream fileOps(pthreadParameters->OperationsPath);

    unsigned __int64 numberOfOperations = NULL;
    unsigned __int64 accountA = NULL;
    unsigned __int64 accountB = NULL;
    unsigned __int64 amount = NULL;

    if (!(fileOps >> numberOfOperations))
    {
        return (DWORD)-1;
    }

    while (numberOfOperations--)
    {
        if (fileOps >> accountA >> accountB >> amount)
        {
            EnterCriticalSection(pthreadParameters->CriticalLock);
            insertInListEntry(pthreadParameters->ListHead, accountA, accountB, amount, pthreadParameters->CriticalLock, pthreadParameters->EventGoOn);
            LeaveCriticalSection(pthreadParameters->CriticalLock);
        }
    }

    //int t = 15;
    //PLIST_ENTRY field = NULL;
    //NODE* currentWork = NULL;
    //while (t--)
    //{
    //    EnterCriticalSection(pthreadParameters->CriticalLock);
    //    field = InterlockedRemoveHeadList(&pthreadParameters->ListHead->ListEntry, pthreadParameters->CriticalLock);
    //    LeaveCriticalSection(pthreadParameters->CriticalLock);

    //    currentWork = (NODE*)CONTAINING_RECORD(field, NODE, ListEntry);
    //    fprintf_s(stdout, "YA");
    //}
    fprintf_s(stdout, "All transfers added to the list\n");

    SetEvent(pthreadParameters->EventTerminate);
    SetEvent(pthreadParameters->EventGoOn);
    return 0;
}