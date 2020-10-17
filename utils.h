#pragma once

#include <stdio.h>
#include "list.h"
#include "banking.h"

// initializes a head to a LIST_ENTRY and returns it
NODE* CreateAList();

// allocates a empty node
NODE* allocateNode();

// inserts a NODE to a LIST_ENTRY
void insertInListEntry(NODE * ListEntryHead, unsigned __int64 AccountA, unsigned __int64 AccountB, unsigned __int64 Amount, CRITICAL_SECTION* CriticalSectionLock, HANDLE EventGoOn);


DWORD WINAPI SetupInitialAccounts(LPVOID lpParam);

DWORD WINAPI ParseOperations(LPVOID lpParam);