#include "banking.h"
#include "utils.h"

#include <sstream>
#include <chrono>
#include <thread>


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
    PBankAccount->logs->push_back("Account created!");
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
            fprintf_s(stdout, "Account %I64u: balance= %I64u\n", it->second->accountIdentifier, it->second->balance);
            std::vector<std::string>::iterator strings;
            /*strings = it->second->logs->begin();
            int index = 0;
            while (strings != it->second->logs->end())
            {
                fprintf_s(stdout, "log: %d - %s\n", index, strings->c_str());
                index++;
                strings++;
            }
            if (NULL != it->second->mutex)
            {
                CloseHandle(it->second->mutex);
            }
            if (NULL != it->second->logs)
            {
                delete it->second->logs;
            }*/
            //free(it->second);
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
    if (NULL != PThreadParameters->EventPauseForCheck)
    {
        CloseHandle(PThreadParameters->EventPauseForCheck);
    }
    if (NULL != PThreadParameters->EventCheckDone)
    {
        CloseHandle(PThreadParameters->EventCheckDone);
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

    std::ostringstream ss;
    ss << "Initial" << " " << Money;
    account->logs->push_back(ss.str());

    return account;
}


BOOL 
CheckIfInBoth(
    std::map<unsigned __int64, PBANK_ACCOUNT>& BankAccounts
)

{
    std::map<unsigned __int64, unsigned __int8> operationsMap;
    std::map<unsigned __int64, unsigned __int8>::iterator opMapIterator;
    std::vector<std::string>::iterator insideLogs;
    std::vector<std::string>::iterator endLogs;
    std::map<unsigned __int64, PBANK_ACCOUNT>::iterator iteratorBank;
    std::string inter;
    unsigned __int8 number = 0;
    unsigned __int64 opID = 0;
    unsigned __int64 acA = 0;
    unsigned __int64 acB = 0;
    unsigned __int64 am = 0;

    
    iteratorBank = BankAccounts.begin();
   

    while (iteratorBank != BankAccounts.end())
    {
        //WaitForSingleObject(iteratorBank->second->mutex, INFINITE);
        insideLogs = iteratorBank->second->logs->begin();
        endLogs = iteratorBank->second->logs->end();

        while (insideLogs != endLogs)
        {
            std::stringstream ss(insideLogs->c_str());
            if (ss >> opID >> acA >> acB >> am)
            {
                opMapIterator = operationsMap.find(opID);
                if (opMapIterator != operationsMap.end())
                {
                    number = opMapIterator->second;
                    number++;
                    opMapIterator->second = number;
                }
                else
                {
                    operationsMap.emplace(opID, (unsigned __int8)1);
                }
            }
            insideLogs++;
        }
        //ReleaseMutex(iteratorBank->second->mutex);
        iteratorBank++;
    }
    opMapIterator = operationsMap.begin();
    while (opMapIterator != operationsMap.end())
    {
        if (opMapIterator->second != 2)
        {
            return FALSE;
        }
        opMapIterator++;
    }
    return TRUE;
}


BOOL 
CheckIfSumsCorrect(
    std::map<unsigned __int64, PBANK_ACCOUNT>& BankAccounts
)

{
    std::map<unsigned __int64, PBANK_ACCOUNT>::iterator iteratorBank;
    std::string inter;
    std::string initialLog;
    unsigned __int8 number = 0;
    unsigned __int64 opID = 0;
    unsigned __int64 acA = 0;
    unsigned __int64 acB = 0;
    unsigned __int64 am = 0;
    unsigned __int64 sumaInitiala = 0;
    unsigned __int64 sumaFinala = 0;

    iteratorBank = BankAccounts.begin();
    while (iteratorBank != BankAccounts.end())
    {
        sumaInitiala = 0;
        sumaFinala = 0;

        std::vector<std::string>::iterator insideLogs;
        insideLogs = iteratorBank->second->logs->begin();

        while (insideLogs != iteratorBank->second->logs->end())
        {
            std::stringstream ss(insideLogs->c_str());
            if (!(ss >> opID >> acA >> acB >> am))
            {
                std::stringstream ss(insideLogs->c_str());
                if (ss >> initialLog >> sumaInitiala)
                {
                    sumaFinala = sumaInitiala;
                }
            }
            else
            {
                if (acA == iteratorBank->second->accountIdentifier)
                {
                    sumaFinala -= am;
                }
                else
                {
                    sumaFinala += am;
                }
            }
            
            if (insideLogs != iteratorBank->second->logs->end())
            {
                insideLogs++;
            }
            
        }
        if (sumaFinala != iteratorBank->second->balance)
        {
            return FALSE;
        }
        iteratorBank++;
        
    }
    return TRUE;
}


BOOL
IntegrityCheck(
    std::map<unsigned __int64, PBANK_ACCOUNT>& BankAccounts
)

{
    return CheckIfInBoth(BankAccounts) & CheckIfSumsCorrect(BankAccounts);
}


DWORD
WINAPI
ThreadIntegrityCheck(
    LPVOID lpParam
)

{
    PTHREAD_PARAMETERS pthread_parameters = (PTHREAD_PARAMETERS)lpParam;
    unsigned __int32 checks = 0;

    while (TRUE)
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(pthread_parameters->ShutDown, 0))
        {
            goto exit;
        }
        EnterCriticalSection(pthread_parameters->CriticalLock);
        pthread_parameters->ThreadsPaused = 0;
        LeaveCriticalSection(pthread_parameters->CriticalLock);

        ResetEvent(pthread_parameters->EventCheckDone);
        ResetEvent(pthread_parameters->EventThreadsPaused);
        SetEvent(pthread_parameters->EventPauseForCheck);

        EnterCriticalSection(pthread_parameters->CriticalLock);
        if (WAIT_OBJECT_0 == WaitForSingleObject(pthread_parameters->ShutDown, 0))
        {
            goto exit;
        }
        LeaveCriticalSection(pthread_parameters->CriticalLock);


        WaitForSingleObject(pthread_parameters->EventThreadsPaused, INFINITE);
        fprintf_s(stdout, "All threads stopped\n");

        if (FALSE == IntegrityCheck(pthread_parameters->BankAccounts))
        {
            fprintf_s(stdout, "Integrity check number %d not passed! Redundant info from now on!\n", checks);
        }
        else
        {
            fprintf_s(stdout, "Integrity check number %d passed, all good!\n", checks);
        }
        SetEvent(pthread_parameters->EventCheckDone);
        checks++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
exit:
    if (FALSE == IntegrityCheck(pthread_parameters->BankAccounts))
    {
        fprintf_s(stdout, "Final integrity check not passed!\n");
        return (DWORD)-1;
    }
    else
    {
        fprintf_s(stdout, "Final integrity check passed! Life's good!\n");
        return TRUE;
    }
}


void
InsertTransferLog(
    unsigned __int64 OperationID,
    unsigned __int64 AccountA,
    unsigned __int64 AccountB,
    unsigned __int64 Amount,
    BOOL Success,
    std::vector<std::string>* LogList
)
{
    if (FALSE == Success)
    {
        std::ostringstream stringStream;
        stringStream << "Transaction number " << OperationID << " failed, insufficient money in account. (transfer amount= " << Amount << ")";
        LogList->push_back(stringStream.str());
    }
    else
    {
        std::ostringstream stringStream;
        stringStream << OperationID << " " << AccountA << " " << AccountB << " " << Amount;
        LogList->push_back(stringStream.str());
    }
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
            InsertTransferLog(OperationID, AccountA, AccountB, Amount, FALSE, pAccountA->logs);
            ReleaseMutex(pAccountA->mutex);
            return;
        }
        pAccountA->balance -= Amount;

        WaitForSingleObject(pAccountB->mutex, INFINITE);

        InsertTransferLog(OperationID, AccountA, AccountB, Amount, TRUE, pAccountA->logs);
        ReleaseMutex(pAccountA->mutex);

        pAccountB->balance += Amount;

        InsertTransferLog(OperationID, AccountA, AccountB, Amount, TRUE, pAccountB->logs);
        ReleaseMutex(pAccountB->mutex);
        fprintf_s(stdout, "Operation number %I64u, %I64u -> %I64u, amount: %I64u\n", OperationID, AccountA, AccountB, Amount);
        break;

    case 1:
        WaitForSingleObject(pAccountB->mutex, INFINITE);

        pAccountB->balance += Amount;

        WaitForSingleObject(pAccountA->mutex, INFINITE);

        if (pAccountA->balance < Amount)
        {
            
            pAccountB->balance -= Amount;
            
            ReleaseMutex(pAccountB->mutex);

            InsertTransferLog(OperationID, AccountA, AccountB, Amount, FALSE, pAccountA->logs);
            ReleaseMutex(pAccountA->mutex);
        }
        else
        {
            InsertTransferLog(OperationID, AccountA, AccountB, Amount, TRUE, pAccountB->logs);
            ReleaseMutex(pAccountB->mutex);

            pAccountA->balance -= Amount;

            InsertTransferLog(OperationID, AccountA, AccountB, Amount, TRUE, pAccountA->logs);
            ReleaseMutex(pAccountA->mutex);
        }
        fprintf_s(stdout, "Operation %I64u, %I64u -> %I64u, amount: %I64u\n", OperationID, AccountA, AccountB, Amount);
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
    HANDLE integrityThread = NULL; // the thread that makes the integrity checks

    HANDLE eventEndOfOperations = NULL;
    HANDLE eventGoOn = NULL;
    HANDLE eventTerminate = NULL;
    HANDLE eventShutdown = NULL;
    HANDLE EventPauseForCheck = NULL;
    HANDLE eventCheckDone = NULL;

    eventEndOfOperations = CreateEvent(NULL, TRUE, FALSE, NULL);
    eventGoOn = CreateEvent(NULL, TRUE, FALSE, NULL);
    eventTerminate = CreateEvent(NULL, TRUE, FALSE, NULL);
    eventShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);
    EventPauseForCheck = CreateEvent(NULL, TRUE, FALSE, NULL);
    eventCheckDone = CreateEvent(NULL, TRUE, FALSE, NULL);

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
    threadParameters.EventPauseForCheck = EventPauseForCheck;
    threadParameters.EventCheckDone = eventCheckDone;
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

    integrityThread = CreateThread(NULL, 0, ThreadIntegrityCheck, &threadParameters, 0, NULL);
    if (NULL == integrityThread)
    {
        return;
    }
   // WaitForSingleObject(operationsThread, INFINITE);

//    int t = 15;
//PLIST_ENTRY field = NULL;
//NODE* currentWork = NULL;
//while (t--)
//{
//    EnterCriticalSection(threadParameters.CriticalLock);
//    field = InterlockedRemoveHeadList(&threadParameters.ListHead->ListEntry,threadParameters.CriticalLock);
//    LeaveCriticalSection(threadParameters.CriticalLock);
//
//    currentWork = (NODE*)CONTAINING_RECORD(field, NODE, ListEntry);
//    fprintf_s(stdout, "YA");
//}

    EnterThreadPool(theListHead, NumberOfThreads, &threadParameters);



    WaitForSingleObject(eventTerminate, INFINITE);
    WaitForSingleObject(eventShutdown, INFINITE);
    WaitForSingleObject(integrityThread, INFINITE);
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
        if (WAIT_OBJECT_0 == WaitForSingleObject(thParam->EventPauseForCheck, 0))
        {
            EnterCriticalSection(thParam->CriticalLock);
            thParam->ThreadsPaused++;
            if (thParam->NumberOfThreads == thParam->ThreadsPaused)
            {
                SetEvent(thParam->EventThreadsPaused);
            }
            LeaveCriticalSection(thParam->CriticalLock);

            fprintf_s(stdout, "A thread paused\n");
            WaitForSingleObject(thParam->EventCheckDone, INFINITE);
        }
        if (InterlockedIsListEmpty(&thParam->ListHead->ListEntry, thParam->CriticalLock, &thParam->EventGoOn))
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
