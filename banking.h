#pragma once

#include <stdio.h>
#include <cstdlib>
#include <map>
#include <vector>
#include <sstream>
#include "list.h"

#define INITIAL_LOGS_NUMBER 10
#define LOG_MAX_LENGTH 400
#define MAX_THREADS 65

typedef struct BANK_ACCOUNT
{
    unsigned __int64          accountIdentifier;
    unsigned __int64          balance;
    HANDLE                    mutex;
    std::vector<std::string>* logs;

}_BANK_ACCOUNT, *PBANK_ACCOUNT;


typedef struct _THREAD_CANDIDATE 
{
    unsigned __int64 accountTransferA;
    unsigned __int64 accountTransferB;
    unsigned __int64 amount;

}THREAD_CANDIDATE, *PTHREAD_CANDIDATE;


typedef struct _NODE
{
    THREAD_CANDIDATE Info;
    LIST_ENTRY ListEntry;

}NODE, *PNODE;


typedef struct _THREAD_PARAMETERS
{
    std::string OperationsPath;
    std::map<unsigned __int64, PBANK_ACCOUNT> BankAccounts;
    unsigned __int64 OperationID;
    DWORD NumberOfThreads;
    NODE* ListHead;
    CRITICAL_SECTION* CriticalLock;
    HANDLE EventEndOfOperations;
    HANDLE EventGoOn;
    HANDLE EventTerminate;
    HANDLE ShutDown;
    BYTE ThreadsEnded;

}THREAD_PARAMETERS, *PTHREAD_PARAMETERS;

PBANK_ACCOUNT CreateBankAccount(unsigned __int64 Id, unsigned __int64 Money);
void StartTheMagic(char* PathTransfers, DWORD threads);
void MakeTransfer(unsigned __int64 OperationID, unsigned __int64 AccountA, unsigned __int64 AccountB, unsigned __int64 Amount, std::map<unsigned __int64, PBANK_ACCOUNT> &BankAccounts);
DWORD WINAPI ThreadDoMagic(LPVOID lpParam);
void EnterThreadPool(NODE* ListEntryHead, DWORD NumberOfThreads, THREAD_PARAMETERS* threadContext);