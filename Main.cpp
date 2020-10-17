#include <stdlib.h>
#include <stdio.h>
#include "banking.h"

int main(int argc, char** argv)
{
    __int64 numberOfThreads = 0;

    if (2 == argc)
    {
        numberOfThreads = 8;
    }
    else if (3 == argc)
    {
        numberOfThreads = atoi(argv[2]);
        if (0 >= numberOfThreads)
        {
            fprintf_s(stderr, "Invalid number of threads. Number must be positive\n");
            return -1;
        }
    }
    else
    {
        fprintf_s(stderr, "Invalid input!\n");
        fprintf_s(stderr, "try: [path of transfers] [number of threads]\n");
        return -1;
    }

    StartTheMagic(argv[1], (DWORD)numberOfThreads);
    system("pause");
    return 0;
}