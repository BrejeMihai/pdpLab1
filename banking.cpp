#include "banking.h"
#include "utils.h"


BOOL 
InitializeBankingAccount(
    PBANK_ACCOUNT PBankAccount
)

{
    PBankAccount->mutex = CreateMutexA(NULL, 0, NULL);
    if (NULL == PBankAccount->mutex)
    {
        return FALSE;
    }
    PBankAccount->logs = new std::vector<std::string>;
    PBankAccount->logs->push_back("STRING");
    return TRUE;
}


void 
DeinitializeBankingAccounts(
    std::map<unsigned __int64, PBANK_ACCOUNT> &BankAccounts
)

{
    std::map<unsigned __int64, PBANK_ACCOUNT>::iterator it;
    it = BankAccounts.begin();
    while (it != BankAccounts.end())
    {
        if (NULL != it->second)
        {
            if (NULL != it->second->mutex)
            {
                CloseHandle(it->second->mutex);
            }
            if (NULL != it->second->logs)
            {
                delete it->second->logs;
            }
            free(it->second);
        }
        it++;
    }
}


void
DeinitializeThreadParameters(
    PTHREAD_PARAMETERS PThreadParameters
)

{
    DeinitializeBankingAccounts(PThreadParameters->BankAccounts);
    if (NULL != PThreadParameters->EventEndOfOperations)
    {
        CloseHandle(PThreadParameters->EventEndOfOperations);
    }
    if (NULL != PThreadParameters->EventGoOn)
    {
        CloseHandle(PThreadParameters->EventGoOn);
    }
    if (NULL != PThreadParameters->EventTerminate)
    {
        CloseHandle(PThreadParameters->EventTerminate);
    }
    if (NULL != PThreadParameters->ShutDown)
    {
        CloseHandle(PThreadParameters->ShutDown);
    }
    if (NULL != PThreadParameters->CriticalLock)
    {
        DeleteCriticalSection(PThreadParameters->CriticalLock);
    }
}


PBANK_ACCOUNT 
CreateBankAccount(
    unsigned __int64 Id,
    unsigned __int64 Money
)

{
    PBANK_ACCOUNT account = (PBANK_ACCOUNT)malloc(sizeof(BANK_ACCOUNT));
    if (FALSE == InitializeBankingAccount(account))
    {
        return NULL;
    }
    account->accountIdentifier = Id;
    account->balance = Money;

    return account;
}


void
MakeTransfer(
    unsigned __int64 OperationID,
    unsigned __int64 AccountA,
    unsigned __int64 AccountB,
    unsigned __int64 Amount,
    std::map<unsigned __int64, PBANK_ACCOUNT> &BankAccounts
)

{
    PBANK_ACCOUNT pAccountA;
    PBANK_ACCOUNT pAccountB;

    if (AccountA == AccountB)
    {
        return;
    }

    int isALowerThanB = AccountA < AccountB;

    std::map<unsigned __int64, PBANK_ACCOUNT>::iterator iterator;
    iterator = BankAccounts.find(AccountA);
    if (iterator == BankAccounts.end())
    {
        return;
    }
    pAccountA = iterator->second;
    
    iterator = BankAccounts.find(AccountB);
    if (iterator == BankAccounts.end())
    {
        return;
    }
    pAccountB = iterator->second;

    switch (isALowerThanB)
    {
    case 0:
        WaitForSingleObject(pAccountA->mutex, INFINITE);

        if (pAccountA->balance < Amount)
        {
            std::ostringstream stringStream;
            stringStream << "Transaction " << OperationID << " failed, insufficient money in account.";

            pAccountA->logs->push_back(stringStream.str());
            ReleaseMutex(pAccountA->mutex);
            return;
        }
        //TODO log sumthg
        pAccountA->balance -= Amount;

        WaitForSingleObject(pAccountB->mutex, INFINITE);

        ReleaseMutex(pAccountA->mutex);

        //TODO log sumthg
        pAccountB->balance += Amount;

        ReleaseMutex(pAccountB->mutex);
        break;

    case 1:
        WaitForSingleObject(pAccountB->mutex, INFINITE);

        pAccountB->balance += Amount;

        WaitForSingleObject(pAccountA->mutex, INFINITE);

        if (pAccountA->balance < Amount)
        {
            std::ostringstream stringStream;
            stringStream << "Transaction " << OperationID << " failed, insufficient money in account.";

            pAccountA->logs->push_back(stringStream.str());
            pAccountB->balance -= Amount;

            ReleaseMutex(pAccountB->mutex);
            ReleaseMutex(pAccountA->mutex);
        }
        else
        {
            ReleaseMutex(pAccountB->mutex);

            pAccountA->balance -= Amount;

            ReleaseMutex(pAccountA->mutex);
        }
        break;
    }
    
}


void 
StartTheMagic(
    char* PathTransfers,
    DWORD NumberOfThreads
)

{
    PNODE theListHead;
    CRITICAL_SECTION criticalLock;
    THREAD_PARAMETERS threadParameters;
    HANDLE initializationThread = NULL; // the thread that reads and initialize the banking accounts
    HANDLE operationsThread = NULL; // the thread that reads and plugs in operations in the list
    HANDLE eventEndOfOperations = NULL;
    HANDLE eventGoOn = NULL;
    HANDLE eventTerminate = NULL;
    HANDLE eventShutdown = NULL;

    eventEndOfOperations = CreateEvent(NULL, TRUE, FALSE, NULL);
    eventGoOn = CreateEvent(NULL, TRUE, FALSE, NULL);
    eventTerminate = CreateEvent(NULL, TRUE, FALSE, NULL);
    eventShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);

    InitializeCriticalSection(&criticalLock);
    theListHead = CreateAList();

    if (NULL == theListHead)
    {
        return;
    }

    threadParameters.OperationsPath = PathTransfers;

    threadParameters.ListHead = theListHead;
    threadParameters.CriticalLock = &criticalLock;
    threadParameters.EventEndOfOperations = eventEndOfOperations;
    threadParameters.EventGoOn = eventGoOn;
    threadParameters.EventTerminate = eventTerminate;
    threadParameters.ShutDown = eventShutdown;
    threadParameters.NumberOfThreads = NumberOfThreads;
    threadParameters.ThreadsEnded = 0;

    initializationThread = CreateThread(NULL, 0, SetupInitialAccounts, &threadParameters, 0, NULL);
    if (NULL == initializationThread)
    {
        return;
    }

    WaitForSingleObject(initializationThread, INFINITE);
    CloseHandle(initializationThread);

    operationsThread = CreateThread(NULL, 0, ParseOperations, &threadParameters, 0, NULL);

    if (NULL == operationsThread)
    {
        return;
    }

    WaitForSingleObject(operationsThread, INFINITE);

    int t = 15;
PLIST_ENTRY field = NULL;
NODE* currentWork = NULL;
while (t--)
{
    EnterCriticalSection(threadParameters.CriticalLock);
    field = InterlockedRemoveHeadList(&threadParameters.ListHead->ListEntry,threadParameters.CriticalLock);
    LeaveCriticalSection(threadParameters.CriticalLock);

    currentWork = (NODE*)CONTAINING_RECORD(field, NODE, ListEntry);
    fprintf_s(stdout, "YA");
}

    EnterThreadPool(theListHead, NumberOfThreads, &threadParameters);

    WaitForSingleObject(eventTerminate, INFINITE);
    WaitForSingleObject(eventShutdown, INFINITE);
    CloseHandle(operationsThread);

    DeinitializeThreadParameters(&threadParameters);
}


void 
EnterThreadPool(
    NODE* ListEntryHead, 
    DWORD NumberOfThreads, 
    THREAD_PARAMETERS* threadContext
)

{
    UNREFERENCED_PARAMETER(ListEntryHead);
    HANDLE  hThreadArray[MAX_THREADS];

    fprintf_s(stdout, "ThreadPool engaged\n");
    for (DWORD i = 0; i < NumberOfThreads; i++)
    {
        hThreadArray[i] = CreateThread(NULL, 0, ThreadDoMagic, threadContext, 0, NULL);
    }

    WaitForSingleObject(threadContext->EventTerminate, INFINITE);
    WaitForSingleObject(threadContext->ShutDown, INFINITE);

    for (DWORD i = 0; i < NumberOfThreads; i++)
    {
        CloseHandle(hThreadArray[i]);
    }

    return;
}


DWORD 
WINAPI 
ThreadDoMagic(
    LPVOID lpParam
)
{
    THREAD_PARAMETERS* thParam = (THREAD_PARAMETERS*)(lpParam);
    PLIST_ENTRY field = NULL;
    NODE* currentWork = NULL;
    unsigned __int64 currentOperationID = NULL;


    while (TRUE)
    {
        if (InterlockedIsListEmpty(&thParam->ListHead->ListEntry, thParam->CriticalLock, thParam->EventGoOn))
        {
            EnterCriticalSection(thParam->CriticalLock);
            if ((WAIT_OBJECT_0 == WaitForSingleObject(thParam->EventTerminate, 0)) && (WAIT_OBJECT_0 != WaitForSingleObject(thParam->EventGoOn, 0)))
            {
                if (WAIT_OBJECT_0 == WaitForSingleObject(thParam->EventTerminate, 0))
                {
                    thParam->ThreadsEnded++;
                    LeaveCriticalSection(thParam->CriticalLock);
                    goto exit;
                }
                else
                {
                    SetEvent(thParam->EventTerminate);
                    thParam->ThreadsEnded++;
                    LeaveCriticalSection(thParam->CriticalLock);
                    goto exit;
                }
            }
            LeaveCriticalSection(thParam->CriticalLock);
            WaitForSingleObject(thParam->EventGoOn, INFINITE);
        }
        else
        {
            EnterCriticalSection(thParam->CriticalLock);
            currentOperationID = thParam->OperationID;
            thParam->OperationID++;
            field = InterlockedRemoveHeadList(&(thParam->ListHead->ListEntry), thParam->CriticalLock);
            LeaveCriticalSection(thParam->CriticalLock);

            currentWork = (NODE*)CONTAINING_RECORD(field, NODE, ListEntry);

            if (currentWork != thParam->ListHead)
            {
                MakeTransfer(currentOperationID, currentWork->Info.accountTransferA, currentWork->Info.accountTransferB, currentWork->Info.amount, thParam->BankAccounts);
            }
            free(currentWork);
        }
    }
    EnterCriticalSection(thParam->CriticalLock);
    thParam->ThreadsEnded++;
    LeaveCriticalSection(thParam->CriticalLock);

exit:
    if (thParam->ThreadsEnded == thParam->NumberOfThreads)
    {
        SetEvent(thParam->ShutDown);
    }
    fprintf_s(stdout, "Peasant life sadly ended :(\n");
    return 0;
}
